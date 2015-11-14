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

#include "vcs.h"

#ifdef HAVE_GIT2
#include <git2.h>
#include <git2/sys/repository.h>
#endif

#include "glog/logging.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"

#include "utils.h"

namespace kythe {

namespace {

#ifdef HAVE_GIT2
std::string OidToString(const ::git_oid &id) {
  char buf[GIT_OID_HEXSZ + 1];
  ::git_oid_tostr(buf, sizeof(buf), &id);
  return std::string(buf);
}

int ReportGit2Error(const char *message, int error) {
  if (error < 0) {
    const auto *err = giterr_last();
    LOG(ERROR) << "while " << message << " libgit2 says " << error << "/"
               << err->klass << ": " << err->message;
  }
  return error;
}

struct AutoGitBuf {
  ::git_buf buf = {0};
  ~AutoGitBuf() { ::git_buf_free(&buf); }
};
#endif

}  // anonymous namespace

VcsClient::VcsClient(const std::string &cache_path) : cache_path_(cache_path) {
#ifdef HAVE_GIT2
  git_libgit2_init();
#endif
  if (auto in_file = OpenFile(cache_path_.c_str(), "r")) {
    char buffer[1024];
    rapidjson::FileReadStream stream(in_file.get(), buffer, sizeof(buffer));
    cache_.ParseStream(stream);
  }
  if (!cache_.IsObject()) {
    LOG(ERROR) << "bad or missing cache at " << cache_path_;
    cache_.Parse("{}");
  }
}

bool VcsClient::GetCacheForUri(const std::string &vcs_uri,
                               rapidjson::Value *val) {
  if (cache_path_.empty()) {
    return false;
  }
  auto member = cache_.FindMember(vcs_uri.c_str());
  if (member != cache_.MemberEnd() && member->value.IsObject()) {
    VLOG(0) << "cache hit for " << vcs_uri;
    *val = member->value;
    return true;
  }
  return false;
}

void VcsClient::UpdateCacheForUri(const std::string &vcs_uri,
                                  rapidjson::Value &val) {
  if (cache_path_.empty()) {
    return;
  }
  auto member = cache_.FindMember(vcs_uri.c_str());
  if (member != cache_.MemberEnd()) {
    cache_.EraseMember(member);
  }
  cache_.AddMember(
      rapidjson::Value(vcs_uri.c_str(), vcs_uri.size(), cache_.GetAllocator()),
      val, cache_.GetAllocator());
  if (auto out_file = OpenFile(cache_path_.c_str(), "w")) {
    char buffer[1024];
    {
      rapidjson::FileWriteStream stream(out_file.get(), buffer, sizeof(buffer));
      rapidjson::Writer<rapidjson::FileWriteStream> writer(stream);
      cache_.Accept(writer);
    }
  } else {
    LOG(ERROR) << "can't write to cache for " << cache_path_;
  }
}

bool VcsClient::GetTags(const std::string &vcs_uri,
                        std::map<std::string, std::string> *tags) {
  rapidjson::Value cached_value;
  if (GetCacheForUri(vcs_uri, &cached_value) && cached_value.IsObject()) {
    auto json_tags = cached_value.FindMember("tags");
    if (json_tags != cached_value.MemberEnd() && json_tags->value.IsObject()) {
      for (auto tag = json_tags->value.MemberBegin();
           tag != json_tags->value.MemberEnd(); ++tag) {
        if (tag->name.IsString() && tag->value.IsString()) {
          tags->insert(
              std::make_pair(tag->name.GetString(), tag->value.GetString()));
        }
      }
      return !tags->empty();
    }
  }
  if (!allow_network_) {
    VLOG(0) << "GetTags request refused network access";
    return false;
  }
  if (!cached_value.IsObject()) {
    cached_value.SetObject();
  }
#ifdef HAVE_GIT2
  ::git_repository *repo = nullptr;
  ::git_remote *remote = nullptr;
  ::git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
  // The memory (possibly) pointed to by heads is owned by remote and will be
  // freed when we free remote.
  const ::git_remote_head **heads = nullptr;
  size_t num_heads = 0;
  if (!ReportGit2Error("git_repository_new", ::git_repository_new(&repo)) &&
      !ReportGit2Error(
          "git_remote_create_anonymous",
          ::git_remote_create_anonymous(&remote, repo, vcs_uri.c_str())) &&
      !ReportGit2Error(
          "git_remote_connect",
          ::git_remote_connect(remote, GIT_DIRECTION_FETCH, &callbacks)) &&
      !ReportGit2Error("git_remote_ls",
                       ::git_remote_ls(&heads, &num_heads, remote))) {
    cached_value.RemoveMember("tags");
    rapidjson::Value json_tags;
    json_tags.SetObject();
    for (size_t h = 0; h < num_heads; ++h) {
      const auto *head = heads[h];
      std::string oid = OidToString(head->oid);
      tags->insert(std::make_pair(head->name, oid));
      json_tags.AddMember(
          rapidjson::Value(head->name, cache_.GetAllocator()),
          rapidjson::Value(oid.c_str(), oid.size(), cache_.GetAllocator()),
          cache_.GetAllocator());
    }
    cached_value.AddMember("tags", json_tags, cache_.GetAllocator());
    UpdateCacheForUri(vcs_uri, cached_value);
  }
  ::git_remote_free(remote);
  ::git_repository_free(repo);
#endif
  return !tags->empty();
}

std::string VcsClient::GetRevisionID(const std::string &path) {
  std::string head_sha;
#ifdef HAVE_GIT2
  AutoGitBuf root;
  ::git_repository *repo = nullptr;
  ::git_reference *href = nullptr;
  ::git_oid oid;
  if (!ReportGit2Error(
          "git_repository_discover",
          git_repository_discover(&root.buf, path.c_str(), 0, NULL)) &&
      !ReportGit2Error("git_repository_open",
                       ::git_repository_open(&repo, root.buf.ptr)) &&
      !ReportGit2Error("git_repository_head",
                       ::git_repository_head(&href, repo)) &&
      !ReportGit2Error("git_reference_name_to_id",
                       ::git_reference_name_to_id(&oid, repo, "HEAD"))) {
    head_sha = OidToString(oid);
  }
  ::git_reference_free(href);
  ::git_repository_free(repo);
#endif
  return head_sha;
}

}  // namespace kythe
