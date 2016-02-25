/*
 * Copyright 2014 Google Inc. All rights reserved.
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

// Allows the Kythe C++ indexer to be invoked from the command line. By default,
// this program reads a single C++ compilation unit from stdin and emits
// binary Kythe artifacts to stdout as a sequence of Entity protos.
// Command-line arguments may be passed to Clang as positional parameters.
//
//   eg: indexer -i foo.cc -o foo.bin -- -DINDEXING
//       indexer -i foo.cc | verifier foo.cc
//       indexer some/index.kindex

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Tooling.h"

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/stubs/common.h"
#include "kythe/cxx/common/index_pack.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/indexing/KytheOutputStream.h"
#include "kythe/cxx/common/indexing/KytheVFS.h"
#include "kythe/cxx/common/json_proto.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/cxx/common/proto_conversions.h"
#include "kythe/proto/analysis.pb.h"
#include "kythe/proto/claim.pb.h"

#include "IndexerFrontendAction.h"
#include "KytheGraphObserver.h"

DEFINE_string(o, "-", "Output filename");
DEFINE_string(i, "-", "Input filename");
DEFINE_bool(ignore_unimplemented, true,
            "Continue indexing even if we find something we don't support.");
DEFINE_bool(flush_after_each_entry, false,
            "Flush output after writing each entry.");
DEFINE_string(static_claim, "", "Use a static claim table.");
DEFINE_bool(claim_unknown, true, "Process files with unknown claim status.");
DEFINE_bool(index_template_instantiations, true,
            "Index template instantiations.");
DEFINE_string(index_pack, "", "Mount an index pack rooted at this directory.");
DEFINE_string(cache, "", "Use a memcache instance (ex: \"--SERVER=foo:1234\")");
DEFINE_int32(min_size, 4096, "Minimum size of an entry bundle");
DEFINE_int32(max_size, 1024 * 32, "Maximum size of an entry bundle");
DEFINE_bool(cache_stats, false, "Show cache stats");
DEFINE_string(icorpus, "", "Corpus to use for files specified with -i");
DEFINE_bool(normalize_file_vnames, false, "Normalize incoming file vnames.");

DEFINE_string(experimental_dynamic_claim_cache, "",
              "Use a memcache instance for dynamic claims (EXPERIMENTAL)");
// Setting this to a value > 1 allows the same object (e.g., a transcript of
// an include file) to be claimed multiple times. In the absence of transcript
// labels, setting this value to 1 means that only one environment will be
// considered when indexing a vname. This may result in (among other effects)
// conditionally included code never being indexed if the symbols checked differ
// between translation units.
DEFINE_uint64(experimental_dynamic_overclaim, 1,
              "Maximum number of dynamic claims per claimable (EXPERIMENTAL)");

namespace kythe {
namespace {
/// \brief Reads the output of the static claim tool.
///
/// `path` should be a file that contains a GZip-compressed sequence of
/// varint-prefixed wire format ClaimAssignment protobuf messages.
void DecodeStaticClaimTable(const std::string &path,
                            kythe::StaticClaimClient *client) {
  using namespace google::protobuf::io;
  int fd = open(path.c_str(), O_RDONLY, S_IREAD | S_IWRITE);
  CHECK_GE(fd, 0) << "Couldn't open input file " << path;
  FileInputStream file_input_stream(fd);
  GzipInputStream gzip_input_stream(&file_input_stream);
  CodedInputStream coded_input_stream(&gzip_input_stream);
  google::protobuf::uint32 byte_size;
  // Silence a warning about input size.
  coded_input_stream.SetTotalBytesLimit(INT_MAX, -1);
  while (coded_input_stream.ReadVarint32(&byte_size)) {
    auto limit = coded_input_stream.PushLimit(byte_size);
    kythe::proto::ClaimAssignment claim;
    CHECK(claim.ParseFromCodedStream(&coded_input_stream));
    // NB: We don't filter on compilation unit here. A dependency has three
    // static states (wrt some CU): unknown, owned by CU, owned by another CU.
    client->AssignClaim(claim.dependency_v_name(), claim.compilation_v_name());
    coded_input_stream.PopLimit(limit);
  }
  close(fd);
}

/// \brief Reads data from a .kindex file into memory.
/// \param path The path from which the file should be read.
/// \param virtual_files A vector to be filled with FileData.
/// \param unit A `CompilationUnit` to be decoded from the .kindex.
void DecodeIndexFile(const std::string &path,
                     std::vector<proto::FileData> *virtual_files,
                     proto::CompilationUnit *unit) {
  using namespace google::protobuf::io;
  int fd = open(path.c_str(), O_RDONLY, S_IREAD | S_IWRITE);
  CHECK_GE(fd, 0) << "Couldn't open input file " << path;
  FileInputStream file_input_stream(fd);
  GzipInputStream gzip_input_stream(&file_input_stream);
  CodedInputStream coded_input_stream(&gzip_input_stream);
  // Silence a warning about input size.
  coded_input_stream.SetTotalBytesLimit(INT_MAX, -1);
  google::protobuf::uint32 byte_size;
  while (coded_input_stream.ReadVarint32(&byte_size)) {
    auto limit = coded_input_stream.PushLimit(byte_size);
    if (unit) {
      CHECK(unit->ParseFromCodedStream(&coded_input_stream));
      unit = nullptr;
    } else {
      proto::FileData content;
      CHECK(content.ParseFromCodedStream(&coded_input_stream));
      CHECK(content.has_info());
      virtual_files->push_back(std::move(content));
    }
    coded_input_stream.PopLimit(limit);
  }
  CHECK(!unit) << "Never saw a CompilationUnit.";
  close(fd);
}

void DecodeIndexPack(const std::string &cu_hash,
                     std::unique_ptr<IndexPack> index_pack,
                     std::vector<proto::FileData> *virtual_files,
                     proto::CompilationUnit *unit) {
  std::string error_text;
  CHECK(index_pack->ReadCompilationUnit(cu_hash, unit, &error_text))
      << "Could not read " << cu_hash << ": " << error_text;
  for (const auto &input : unit->required_input()) {
    const auto &info = input.info();
    CHECK(!info.path().empty());
    CHECK(!info.digest().empty()) << "Required input " << info.path()
                                  << " is missing its digest.";
    std::string read_data;
    CHECK(index_pack->ReadFileData(info.digest(), &read_data))
        << "Could not read " << info.path() << " (digest " << info.digest()
        << ") from the index pack: " << read_data;
    proto::FileData file_data;
    file_data.set_content(read_data);
    file_data.mutable_info()->set_path(info.path());
    file_data.mutable_info()->set_digest(info.digest());
    virtual_files->push_back(std::move(file_data));
  }
}

/// \brief Does `input` end with `suffix`?
bool EndsWith(const std::string &input, const std::string &suffix) {
  return input.size() >= suffix.size() &&
         !input.compare(input.size() - suffix.size(), suffix.size(), suffix);
}
}  // anonymous namespace

int main(int argc, char *argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::InitGoogleLogging(argv[0]);
  google::SetVersionString("0.1");
  google::SetUsageMessage(R"(Command-line frontend for the Kythe C++ indexer.
Invokes the Kythe C++ indexer on a single compilation unit. By default reads
source text from stdin and writes binary Kythe artifacts to stdout as a sequence
of Entity protos. Command-line arguments may be passed to Clang as positional
parameters.

If -index_pack is not specified, there may be a positional parameter specified
that ends in .kindex. If one exists, no other positional parameters may be
specified, nor may an additional input parameter be specified. Input will
be read from the index file.

If -index_pack is specified, there must be exactly one positional parameter.
This parameter should be the compilation unit ID from the mounted index pack
that is meant to be indexed. No additional input parameters may be specified.

Examples:
  indexer -index_pack path/to/pack/root 660f1f840000000000
  indexer some/index.kindex
  indexer -i foo.cc -o foo.bin -- -DINDEXING
  indexer -i foo.cc | verifier foo.cc")");
  google::ParseCommandLineFlags(&argc, &argv, true);

  std::vector<std::string> final_args(argv, argv + argc);

  // Check to see if we should be using an index pack or a .kindex file.
  std::unique_ptr<kythe::IndexPack> index_pack;
  std::string kindex_file_or_cu;
  if (!FLAGS_index_pack.empty()) {
    std::string error_text;
    auto filesystem = kythe::IndexPackPosixFilesystem::Open(
        FLAGS_index_pack, kythe::IndexPackFilesystem::OpenMode::kReadOnly,
        &error_text);
    CHECK(filesystem) << "Couldn't open index pack from " << FLAGS_index_pack
                      << ": " << error_text;
    index_pack.reset(new kythe::IndexPack(std::move(filesystem)));
    CHECK(final_args.size() >= 2) << "You must specify a compilation unit.";
    kindex_file_or_cu = final_args[1];
  } else {
    std::string kindex_suffix = ".kindex";
    for (const auto &arg : final_args) {
      if (EndsWith(arg, kindex_suffix)) {
        kindex_file_or_cu = arg;
        break;
      }
    }
  }

  final_args.erase(
      std::remove(final_args.begin(), final_args.end(), std::string()),
      final_args.end());

  if (!kindex_file_or_cu.empty()) {
    CHECK_EQ(2, final_args.size())
        << "No other positional arguments are allowed when reading "
        << "from an index file or an index pack.";
    CHECK_EQ("-", FLAGS_i)
        << "No other input is allowed when reading from an index file or an "
        << "index pack.";
  }

  std::vector<proto::FileData> virtual_files;
  proto::CompilationUnit unit;
  std::string working_dir;

  if (!kindex_file_or_cu.empty()) {
    if (index_pack) {
      DecodeIndexPack(kindex_file_or_cu, std::move(index_pack), &virtual_files,
                      &unit);
    } else {
      DecodeIndexFile(kindex_file_or_cu, &virtual_files, &unit);
    }
    working_dir = unit.working_directory();
    if (!llvm::sys::path::is_absolute(working_dir)) {
      llvm::SmallString<1024> stored_wd;
      CHECK(!llvm::sys::fs::make_absolute(stored_wd));
      working_dir = stored_wd.str();
    }
  } else {
    int read_fd = STDIN_FILENO;
    std::string source_file_name = "stdin.cc";
    llvm::SmallString<1024> cwd;
    CHECK(!llvm::sys::fs::current_path(cwd));
    working_dir = cwd.str();

    if (FLAGS_i != "-") {
      read_fd = open(FLAGS_i.c_str(), O_RDONLY);
      if (read_fd == -1) {
        perror("Can't open input file");
        exit(1);
      }
      source_file_name = FLAGS_i;
    }

    final_args.push_back(source_file_name);

    char buf[1024];
    llvm::SmallString<1024> source_data;
    ssize_t amount_read;
    while ((amount_read = read(read_fd, buf, 1024)) > 0) {
      source_data.append(llvm::StringRef(buf, amount_read));
    }
    if (amount_read < 0) {
      perror("Error reading input file");
      exit(1);
    }
    close(read_fd);
    // clang wants the source file to be null-terminated, but this should
    // not be in range of the StringRef. std::string ends with \0.
    proto::FileData file_data;
    file_data.mutable_info()->set_path(source_file_name);
    file_data.set_content(source_data.str());
    virtual_files.push_back(std::move(file_data));
    for (const auto &arg : final_args) {
      unit.add_argument(arg);
    }
    unit.mutable_v_name()->set_corpus(FLAGS_icorpus);
  }

  std::unique_ptr<kythe::KytheClaimClient> claim_client;
  if (!FLAGS_experimental_dynamic_claim_cache.empty()) {
    auto dynamic_claims = std::unique_ptr<kythe::DynamicClaimClient>(
        new kythe::DynamicClaimClient());
    dynamic_claims->set_max_redundant_claims(
        FLAGS_experimental_dynamic_overclaim);
    if (!dynamic_claims->OpenMemcache(FLAGS_experimental_dynamic_claim_cache)) {
      fprintf(stderr, "Can't open memcached\n");
      exit(1);
    }
    claim_client.reset(dynamic_claims.release());
  } else {
    auto static_claims = std::unique_ptr<kythe::StaticClaimClient>(
        new kythe::StaticClaimClient());
    if (!FLAGS_static_claim.empty()) {
      DecodeStaticClaimTable(FLAGS_static_claim, static_claims.get());
    }
    static_claims->set_process_unknown_status(FLAGS_claim_unknown);
    claim_client.reset(static_claims.release());
  }

  int write_fd = STDOUT_FILENO;
  if (FLAGS_o != "-") {
    write_fd = open(FLAGS_o.c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (write_fd == -1) {
      perror("Can't open output file");
      exit(1);
    }
  }

  std::string result;

  {
    google::protobuf::io::FileOutputStream raw_output(write_fd);
    kythe::FileOutputStream kythe_output(&raw_output);
    kythe_output.set_show_stats(FLAGS_cache_stats);
    kythe_output.set_flush_after_each_entry(FLAGS_flush_after_each_entry);
    kythe::MemcachedHashCache MHashCache;
    if (!FLAGS_cache.empty()) {
      if (!MHashCache.OpenMemcache(FLAGS_cache)) {
        fprintf(stderr, "Can't open memcached\n");
        exit(1);
      }
      MHashCache.SetSizeLimits(FLAGS_min_size, FLAGS_max_size);
    }

    IndexerOptions options;
    options.TemplateBehavior = FLAGS_index_template_instantiations
                                   ? BehaviorOnTemplates::VisitInstantiations
                                   : BehaviorOnTemplates::SkipInstantiations;
    options.UnimplementedBehavior =
        FLAGS_ignore_unimplemented ? kythe::BehaviorOnUnimplemented::Continue
                                   : kythe::BehaviorOnUnimplemented::Abort;
    options.AllowFSAccess = kindex_file_or_cu.empty();
    options.EnableLossyClaiming =
        FLAGS_experimental_dynamic_claim_cache.empty() ? false : true;
    options.EffectiveWorkingDirectory = working_dir;

    if (FLAGS_normalize_file_vnames) {
      for (auto &input : *unit.mutable_required_input()) {
        input.mutable_v_name()->set_path(
            CleanPath(ToStringRef(input.v_name().path())));
        input.mutable_v_name()->clear_signature();
      }
    }

    kythe::MetadataSupports meta_supports;
    meta_supports.push_back(llvm::make_unique<KytheMetadataSupport>());

    result = IndexCompilationUnit(unit, virtual_files, *claim_client,
                                  FLAGS_cache.empty() ? nullptr : &MHashCache,
                                  kythe_output, options, &meta_supports);
  }

  if (close(write_fd) != 0) {
    perror("Error closing output file");
    exit(1);
  }

  if (!result.empty()) {
    fprintf(stderr, "Error: %s\n", result.c_str());
  }

  return !result.empty();
}

}  // namespace kythe

int main(int argc, char *argv[]) { return kythe::main(argc, argv); }
