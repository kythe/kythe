/*
 * Copyright 2015 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "meta_index.h"

#include <sys/stat.h>
#include <fcntl.h>

#include <queue>

#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/proto/storage.pb.h"
#include "llvm/Support/Path.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "re2/re2.h"

#include "utils.h"

namespace kythe {

namespace {
template <typename M>
void WriteLengthDelimited(google::protobuf::io::ZeroCopyOutputStream *stream,
                          const M &message) {
  google::protobuf::io::CodedOutputStream coded_stream(stream);
  coded_stream.WriteVarint32(message.ByteSize());
  message.SerializeToCodedStream(&coded_stream);
}

std::string GetCorpus(const std::string &vcs_type, const std::string &vcs_uri,
                      const std::string &vcs_id, const std::string &vcs_path) {
  // We don't seem to like corpora with special characters? We should be able
  // to use "/" and not "_" and get rid of the escaping loop.
  std::string corpus = vcs_uri + "_" + vcs_id;
  for (size_t i = 0; i < corpus.size(); ++i) {
    if (corpus[i] == '/') corpus[i] = '_';
    if (corpus[i] == ':') corpus[i] = '_';
  }
  return corpus;
}

/// \brief Quotes `to_quote` as a shell argument.
std::string ShellQuote(const std::string &to_quote) {
  std::string quoted;
  quoted.reserve(to_quote.size() + 2);
  quoted.push_back('\'');
  for (const auto &ch : to_quote) {
    if (ch == '\'') {
      quoted.append("'\''");
    } else {
      quoted.push_back(ch);
    }
  }
  quoted.push_back('\'');
  return quoted;
}
}

Repositories::Repositories(
    llvm::IntrusiveRefCntPtr<clang::vfs::FileSystem> filesystem,
    std::unique_ptr<CommonJsPackageRegistryClient> registry,
    std::unique_ptr<VcsClient> vcs)
    : filesystem_(filesystem),
      registry_(std::move(registry)),
      vcs_(std::move(vcs)) {
  auto cwd = filesystem_->getCurrentWorkingDirectory();
  if (!cwd) {
    LOG(FATAL) << "Filesystem has no working directory set.";
  }
}

// TODO(zarko): If we end up with more than a couple kinds of repositories
// we want to support--and I suspect we will--we should set aside some time
// to design a tiny language for specifying repository traits.

// TODO(zarko): add node .d.ts if it's a node repository

// TODO(zarko): consider making smaller vnameses.json if it turns out that
// doing all the matching is too slow

void Repositories::ExportIndexScript(const llvm::Twine &path,
                                     const llvm::Twine &vnames_json,
                                     const llvm::Twine &subgraph) {
  auto out_file = OpenFile(path.str().c_str(), "w");
  if (!out_file) {
    LOG(ERROR) << "can't open " << path.str();
    return;
  }
  auto write = [&out_file](const char *value) {
    ::fputs(value, out_file.get());
  };

  ::fprintf(out_file.get(), R"zz(#!/bin/bash
: "${KYTHE_GRAPHSTORE:?No KYTHE_GRAPHSTORE specified.}"
KYTHE_GRAPHSTORE="$(realpath ${KYTHE_GRAPHSTORE})"
export KYTHE_VNAMES="%s"
KYTHE_SUBGRAPH="%s"
)zz",
            vnames_json.str().c_str(), subgraph.str().c_str());

  for (const auto &repo : repositories_) {
    std::vector<std::string> index_args = repo->GetIndexArgs();
    if (index_args.empty()) {
      continue;
    }
    write("(cd kythe/ts && \\\n");
    write("  node build/lib/main.js --skipDefaultLib \\\n");
    for (const auto &arg : index_args) {
      ::fprintf(out_file.get(), "    %s \\\n", ShellQuote(arg).c_str());
    }
    write(" | /opt/kythe/tools/entrystream --read_json \\\n");
    write(" | /opt/kythe/tools/write_entries -graphstore \\\n");
    write("    \"${KYTHE_GRAPHSTORE}\")\n");
  }

  write(R"(
/opt/kythe/tools/write_entries -graphstore "${KYTHE_GRAPHSTORE}" \
    < "${KYTHE_SUBGRAPH}"
if [[ ! -z "${KYTHE_TABLES}" ]]; then
  /opt/kythe/tools/write_tables -graphstore "${KYTHE_GRAPHSTORE}" \
      -out="${KYTHE_TABLES}"
  if [[ ! -z "${KYTHE_LISTEN_AT}" ]]; then
    /opt/kythe/tools/http_server -serving_table "${KYTHE_TABLES}" \
        -public_resources="/opt/kythe/web/ui" \
        -listen="${KYTHE_LISTEN_AT}"
  fi
fi
exit 0
)");
}

void Repositories::ExportVNamesJson(const llvm::Twine &path) {
  auto out_file = OpenFile(path.str().c_str(), "w");
  if (!out_file) {
    LOG(ERROR) << "can't open " << path.str();
    return;
  }
  char buffer[1024];
  {
    rapidjson::FileWriteStream stream(out_file.get(), buffer, sizeof(buffer));
    rapidjson::Writer<rapidjson::FileWriteStream> writer(stream);
    WriteVNamesJson(&writer);
  }
}

void Repositories::ExportRepoSubgraph(const llvm::Twine &path) {
  int out_fd = ::open(path.str().c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                      S_IREAD | S_IWRITE);
  if (out_fd < 0) {
    LOG(ERROR) << "can't open " << path.str();
    return;
  }
  {
    namespace io = google::protobuf::io;
    io::FileOutputStream file_output_stream(out_fd);
    for (const auto &repo : repositories_) {
      repo->EmitKytheSubgraph(&file_output_stream);
    }
    if (!file_output_stream.Close()) {
      LOG(ERROR) << "can't close " << path.str();
    }
  }
}

Repository::PathMatchingRule Repository::GenerateGenericPathMatchingRule(
    const std::string &local_repo_path, const std::string &vcs_type,
    const std::string &vcs_uri, const std::string &vcs_id,
    const std::string &vcs_path, size_t match_priority) {
  Repository::PathMatchingRule rule;
  rule.match_priority = match_priority;
  rule.match_regex = RE2::QuoteMeta(local_repo_path + "/") + "(.*)";
  rule.corpus_template = GetCorpus(vcs_type, vcs_uri, vcs_id, vcs_path);
  rule.path_template = "@1@";
  return rule;
}

template <typename W>
void Repositories::WriteVNamesJson(W *writer) {
  writer->StartArray();
  std::priority_queue<Repository::PathMatchingRule> rules;
  for (const auto &repo : repositories_) {
    rules.emplace(repo->path_matching_rule());
  }
  while (!rules.empty()) {
    const auto &rule = rules.top();
    writer->StartObject();
    writer->Key("pattern");
    writer->String(rule.match_regex.c_str());
    writer->Key("vname");
    writer->StartObject();
    writer->Key("corpus");
    writer->String(rule.corpus_template.c_str());
    writer->Key("path");
    writer->String(rule.path_template.c_str());
    if (!rule.root_template.empty()) {
      writer->Key("path");
      writer->String(rule.root_template.c_str());
    }
    writer->EndObject();  // vname object
    writer->EndObject();  // rule object
    rules.pop();
  }
  writer->EndArray();  // rule array
}

void Repositories::AddRepositoryRoot(const llvm::Twine &path) {
  std::vector<WorkItem> worklist = {{0, path.str()}};
  CrawlRepositories(&worklist);
}

/// \brief A Repository exposing a node.js project.
class NodeJsRepository : public Repository {
 public:
  PathMatchingRule path_matching_rule() const { return path_matching_rule_; }
  void EmitKytheSubgraph(
      google::protobuf::io::ZeroCopyOutputStream *stream) const {
    kythe::proto::Entry entry;
    entry.mutable_source()->set_corpus(
        GetCorpus(vcs_type_, vcs_uri_, vcs_id_, vcs_path_));
    entry.set_fact_name("/kythe/node/kind");
    entry.set_fact_value("vcs");
    WriteLengthDelimited(stream, entry);
    entry.set_fact_name("/kythe/vcs/type");
    entry.set_fact_value(vcs_type_);
    WriteLengthDelimited(stream, entry);
    entry.set_fact_name("/kythe/vcs/uri");
    entry.set_fact_value(vcs_uri_);
    WriteLengthDelimited(stream, entry);
    entry.set_fact_name("/kythe/vcs/id");
    entry.set_fact_value(vcs_id_);
    WriteLengthDelimited(stream, entry);
    if (!vcs_path_.empty()) {
      entry.set_fact_name("/kythe/vcs/path");
      entry.set_fact_value(vcs_path_);
      WriteLengthDelimited(stream, entry);
    }
  }

  std::vector<std::string> GetIndexArgs() const { return indexing_args_; }

 private:
  NodeJsRepository(const llvm::Twine &root, const llvm::Twine &default_vcs_id)
      : root_(root.str()), default_vcs_id_(default_vcs_id.str()) {}
  friend class Repositories;

  /// \brief Parse the package manifest, extracting any identifying information.
  ///
  /// This function parses `package_` from JSON in `manifest_data`. It
  /// attempts to discover the values for `npm_id_`, `commonjs_id_`, and
  /// `commonjs_version_`.
  ///
  /// \return false if the manifest was structurally invalid; true if we may
  /// be able to index the package.
  bool ParseManifest(const char *manifest_data) {
    package_.Parse(manifest_data);
    if (package_.HasParseError()) {
      LOG(WARNING) << "couldn't parse package.json: "
                   << rapidjson::GetParseError_En(package_.GetParseError())
                   << " near " << package_.GetErrorOffset();
      return false;
    }
    if (!package_.IsObject()) {
      LOG(WARNING) << root_.str().str()
                   << ": package.json doesn't describe an object.";
      return false;
    }
    const auto npm_id = package_.FindMember("_id");
    const auto package_id = package_.FindMember("name");
    const auto package_version = package_.FindMember("version");
    if (npm_id != package_.MemberEnd() && npm_id->value.IsString()) {
      VLOG(0) << root_.str().str()
              << ": package.json has an _id; assuming it's npm's.";
      npm_id_ = npm_id->value.GetString();
    }
    if (package_id != package_.MemberEnd() && package_id->value.IsString()) {
      commonjs_id_ = package_id->value.GetString();
    }
    if (package_version != package_.MemberEnd() &&
        package_version->value.IsString()) {
      commonjs_version_ = package_version->value.GetString();
    }
    return true;
  }

  /// \brief Attempt to find VCS identifiers for this package.
  /// \pre ParseManifest has been called.
  /// \param registry The registry to use to look up identifiers.
  /// \param vcs The VCS to use to look up identifiers for local packages or
  /// for packages that `registry` can't provide enough information about.
  /// \return true if we have enough identifying information to be able to index
  /// this package; false if we will not be able to index it.
  bool DetectVersionControlIdentifiers(CommonJsPackageRegistryClient *registry,
                                       VcsClient *vcs) {
    if (npm_id_.empty()) {
      // This probably wasn't installed by npm, so we'll try it as a local repo.
      if (!AutodetectVersionControlIdentifiers(vcs)) {
        return false;
      }
    } else {
      // Looks like this came from npm.
      const auto package_repo = package_.FindMember("repository");
      if (package_repo == package_.MemberEnd() ||
          !DecodeRepository(package_repo->value)) {
        LOG(WARNING) << root_.str().str() << ": bad local repository";
        return false;
      }
      if (!QueryRegistryForVersionControlIdentifiers(registry, vcs)) {
        return false;
      }
    }
    return true;
  }

  /// \brief Add dependencies of this package to the worklist.
  /// \pre DetectVersionControlIdentifiers has been called.
  /// \param filesystem The filesystem to use to look for dependencies.
  /// \param current_depth The depth of this repository, used for ranking
  /// path matching rules.
  /// \param worklist The worklist to add new dependencies onto.
  /// \return true if we can continue; false if this package has unrecoverable
  /// errors.
  bool EnqueueDependencies(clang::vfs::FileSystem *filesystem,
                           size_t current_depth,
                           std::vector<Repositories::WorkItem> *worklist) {
    // Try to find the node_modules directory.
    llvm::SmallString<1024> node_modules_path;
    llvm::sys::path::append(node_modules_path, root_, "node_modules");
    auto modules_dir = filesystem->status(node_modules_path);
    if (auto err = modules_dir.getError()) {
      LOG(WARNING) << node_modules_path.str().str() << ": no node_modules";
    } else {
      node_modules_ = *modules_dir;
      const auto dependencies = package_.FindMember("dependencies");
      if (dependencies != package_.MemberEnd() &&
          dependencies->value.IsObject()) {
        EnqueueDependencies(dependencies->value, current_depth, worklist);
      }
      const auto devDeps = package_.FindMember("devDependencies");
      if (devDeps != package_.MemberEnd() && devDeps->value.IsObject()) {
        EnqueueDependencies(devDeps->value, current_depth, worklist);
      }
    }
    return true;
  }

  /// Parse the package manifest, adding any discovered dependencies to the
  /// worklist. Returns true if this repository should be retained.
  /// A repository can be retained if we can give it an identifier.
  /// \brief Computes the actions and rules necessary to index this repository.
  /// \pre DetectVersionControlIdentifiers has been called.
  /// \param current_depth The depth of this repository, used for ranking
  /// path matching rules.
  /// \param filesystem The filesystem to use to look for source files.
  /// \return true if this repository should be saved and indexed; false if
  /// it should be disposed.
  bool ComputeIndexingActions(size_t current_depth,
                              clang::vfs::FileSystem *filesystem) {
    // Now we've figured out our VCS identifiers. Identify ourselves and crawl
    // dependent packages.
    ComputePathMatchingRule(current_depth);
    // Finally, identify the files in the repository that we'll want to index.
    indexing_args_.push_back("--rootDir");
    indexing_args_.push_back(root_.str());
    indexing_args_.push_back("--");
    // This is a hack to get commonjs modules working.
    indexing_args_.push_back("implicit=typings/node/node.d.ts");
    SearchForSource(filesystem, root_, &indexing_args_);
    return true;
  }

  void EnqueueDependencies(const rapidjson::Value &dependencies,
                           size_t current_depth,
                           std::vector<Repositories::WorkItem> *worklist) {
    for (auto i = dependencies.MemberBegin(); i != dependencies.MemberEnd();
         i++) {
      if (i->name.IsString()) {
        llvm::SmallString<1024> new_path;
        llvm::sys::path::append(new_path, node_modules_.getName(),
                                i->name.GetString());
        worklist->push_back({current_depth + 1, new_path.str()});
      }
    }
  }

  /// \brief Attempt to determine identifying information from a repository.
  /// \param value A repository-shaped value.
  /// \return true if we found the information we need.
  bool DecodeRepository(const rapidjson::Value &value) {
    if (value.IsString()) {
      // TODO(zarko): support string values here
      LOG(WARNING) << root_.str().str()
                   << ": can't deal with string repositories";
      return false;
    }
    if (!value.IsObject()) {
      LOG(WARNING) << root_.str().str()
                   << ": can't deal with non-object repositories";
      return false;
    }
    // 'Each repository is a hash with properties for the "type" and "url"
    // location of the repository to clone/checkout the package.'
    const auto repo_type = value.FindMember("type");
    const auto repo_url = value.FindMember("url");
    if (repo_type == value.MemberEnd() || !repo_type->value.IsString() ||
        repo_url == value.MemberEnd() || !repo_url->value.IsString()) {
      LOG(WARNING) << root_.str().str() << ": missing repository type or url";
      return false;
    }
    vcs_type_ = repo_type->value.GetString();
    vcs_uri_ = repo_url->value.GetString();
    // 'A "path" property may also be specified to locate the package in the
    // repository if it does not reside at the root.'
    const auto repo_path = value.FindMember("path");
    if (repo_path != value.MemberEnd() && repo_path->value.IsString()) {
      vcs_path_ = repo_path->value.GetString();
    }
    return true;
  }
  void SearchForSource(clang::vfs::FileSystem *filesystem,
                       const llvm::Twine &path,
                       std::vector<std::string> *arg_list) const {
    std::error_code err;
    for (auto it = filesystem->dir_begin(path, err);
         it != clang::vfs::directory_iterator(); it = it.increment(err)) {
      if (err) {
        LOG(ERROR) << root_.str().str() << ": error (" << path.str()
                   << "): " << err.message();
        return;
      }
      // Don't walk across repositories.
      if (it->getName().endswith("node_modules")) {
        continue;
      }
      if (it->getType() == llvm::sys::fs::file_type::directory_file) {
        SearchForSource(filesystem, it->getName(), arg_list);
      } else if (it->getName().endswith(".js") ||
                 it->getName().endswith(".ts")) {
        arg_list->push_back(it->getName().str());
      }
    }
  }
  /// \brief If this package came from the registry, we can ask the registry
  /// for its repo provenance.
  /// \pre DecodeRepository has passed.
  bool QueryRegistryForVersionControlIdentifiers(
      CommonJsPackageRegistryClient *registry, VcsClient *vcs) {
    if (commonjs_id_.empty() || commonjs_version_.empty()) {
      LOG(WARNING) << root_.str().str()
                   << ": can't query the registry without id or version";
      return false;
    }
    rapidjson::Document repo_metadata;
    if (!registry->GetPackage(commonjs_id_, &repo_metadata)) {
      LOG(WARNING) << root_.str().str() << ": can't find package "
                   << commonjs_id_;
      return false;
    }
    if (repo_metadata.IsObject()) {
      const auto versions = repo_metadata.FindMember("versions");
      if (versions != repo_metadata.MemberEnd() && versions->value.IsObject()) {
        const auto version =
            versions->value.FindMember(commonjs_version_.c_str());
        if (version == versions->value.MemberEnd() ||
            !version->value.IsObject()) {
          LOG(WARNING) << root_.str().str() << ": can't find " << commonjs_id_
                       << "@" << commonjs_version_;
          return false;
        }
        const auto repo = version->value.FindMember("repository");
        if (repo == version->value.MemberEnd() ||
            !DecodeRepository(repo->value)) {
          LOG(WARNING) << root_.str().str() << ": bad repository for "
                       << commonjs_id_ << "@" << commonjs_version_;
          return false;
        }
        const auto git_head = version->value.FindMember("gitHead");
        if (git_head == version->value.MemberEnd() ||
            !git_head->value.IsString()) {
          LOG(WARNING) << root_.str().str() << ": no gitHead for " << vcs_uri_
                       << "/" << commonjs_id_ << "@" << commonjs_version_;
          std::map<std::string, std::string> tags;
          if (vcs->GetTags(vcs_uri_, &tags)) {
            auto tag = tags.find("refs/tags/v" + commonjs_version_);
            if (tag == tags.end()) {
              tag = tags.find("refs/tags/" + commonjs_version_);
            }
            if (tag != tags.end()) {
              vcs_id_ = tag->second;
              LOG(INFO) << root_.str().str() << ": pulled " << vcs_id_
                        << " from the repository";
              return true;
            }
          }
          return false;
        }
        vcs_id_ = git_head->value.GetString();
        return true;
      }
    }
    LOG(WARNING) << root_.str().str() << ": bad package metadata";
    return false;
  }
  /// \brief If this package didn't come from the registry, we can still try to
  /// detect its repo provenance using the local file system.
  bool AutodetectVersionControlIdentifiers(VcsClient *vcs) {
    // NB: if you're stubbing out the filesystem with a VFS, it needs to be
    // in cahoots with vcs_.
    vcs_id_ = vcs->GetRevisionID(root_.str());
    if (vcs_id_.empty()) {
      LOG(WARNING) << root_.str().str() << ": couldn't find a git root";
      if (!default_vcs_id_.empty()) {
        LOG(WARNING) << root_.str().str() << ": using default ID "
                     << default_vcs_id_;
        vcs_id_ = default_vcs_id_;
        return true;
      }
      return false;
    }
    LOG(INFO) << root_.str().str() << ": found git root; HEAD is " << vcs_id_;
    return true;
  }
  /// \brief Uses repository identity information to compute a path matching
  /// rule for this host.
  void ComputePathMatchingRule(size_t search_depth) {
    path_matching_rule_ = GenerateGenericPathMatchingRule(
        root_.str(), vcs_type_, vcs_uri_, vcs_id_, vcs_path_, search_depth);
  }
  /// The status of the node_modules directory, if one exists.
  clang::vfs::Status node_modules_;
  /// Root of the repository. Paths are normalized from here.
  llvm::SmallString<1024> root_;
  /// Indexing tasks for this repository (just an arguments list now).
  std::vector<std::string> indexing_args_;
  /// package.json.
  rapidjson::Document package_;
  /// If this package was installed from npm, it's npm's identifier for it.
  std::string npm_id_;
  /// This package's CommonJS version.
  std::string commonjs_version_;
  /// This package's CommonJS id.
  std::string commonjs_id_;
  /// This package's VCS id (e.g., Git commit sha)
  std::string vcs_id_;
  /// This package's VCS URI (e.g.., https://foo.bar/baz.git)
  std::string vcs_uri_;
  /// This package's VCS path (relative to repo root; may be empty)
  std::string vcs_path_;
  /// This package's VCS type (e.g., "git")
  std::string vcs_type_;
  /// The VCS id to use if we can't find one for a local path.
  std::string default_vcs_id_;
  /// This repository's path matching rule.
  PathMatchingRule path_matching_rule_;
};

void Repositories::CrawlRepositories(std::vector<WorkItem> *worklist) {
  while (!worklist->empty()) {
    auto work_item = worklist->back();
    worklist->pop_back();
    if (!visited_.insert(work_item.root_path).second) {
      continue;
    }
    llvm::SmallString<1024> path(work_item.root_path);
    if (auto err = filesystem_->makeAbsolute(path)) {
      LOG(WARNING) << "failed to make absolute path: ", err.message();
      continue;
    }
    // Try to find and parse a 'package.json'.
    llvm::SmallString<1024> package_json_path;
    llvm::sys::path::append(package_json_path, path, "package.json");
    // NB: getBufferForFile null-terminates by default.
    auto package_json = filesystem_->getBufferForFile(package_json_path);
    if (auto err = package_json.getError()) {
      continue;
    }
    VLOG(0) << "crawl(" << work_item.root_path << ")";
    std::unique_ptr<NodeJsRepository> node_js(
        new NodeJsRepository(path, default_vcs_id_));
    if (node_js->ParseManifest((*package_json)->getBuffer().data()) &&
        node_js->DetectVersionControlIdentifiers(registry_.get(), vcs_.get()) &&
        node_js->EnqueueDependencies(filesystem_.get(), work_item.search_depth,
                                     worklist) &&
        node_js->ComputeIndexingActions(work_item.search_depth,
                                        filesystem_.get())) {
      repositories_.emplace_back(std::move(node_js));
    }
  }
}
}
