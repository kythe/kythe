/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/indexer/proto/source_tree.h"

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "google/protobuf/stubs/map_util.h"
#include "kythe/cxx/common/path_utils.h"

namespace kythe {

using ::google::protobuf::FindOrDie;
using ::google::protobuf::FindOrNull;
using ::google::protobuf::InsertIfNotPresent;

namespace {

// TODO(justbuchanan): why isn't there a replace_all=false version of
// StrReplace() in open-source abseil?
/// Finds the first occurrence of @oldsub in @s and replaces it with @newsub. If
/// @oldsub is not present, just returns @s.
std::string StringReplaceFirst(absl::string_view s, absl::string_view oldsub,
                               absl::string_view newsub) {
  return absl::StrJoin(absl::StrSplit(s, absl::MaxSplits(oldsub, 1)), newsub);
}

}  // namespace

bool PreloadedProtoFileTree::AddFile(const std::string& filename,
                                     const std::string& contents) {
  VLOG(1) << filename << " added to PreloadedProtoFileTree";
  return InsertIfNotPresent(&file_map_, filename, contents);
}

google::protobuf::io::ZeroCopyInputStream* PreloadedProtoFileTree::Open(
    const std::string& filename) {
  last_error_ = "";

  const std::string* cached_path = FindOrNull(*file_mapping_cache_, filename);
  if (cached_path != nullptr) {
    std::string* stored_contents = FindOrNull(file_map_, *cached_path);
    if (stored_contents == nullptr) {
      last_error_ = "Proto file Open(" + filename +
                    ") failed:" + " cached mapping to " + *cached_path +
                    "no longer valid.";
      LOG(ERROR) << last_error_;
      return nullptr;
    }
    return new google::protobuf::io::ArrayInputStream(stored_contents->data(),
                                                      stored_contents->size());
  }
  for (auto& substitution : *substitutions_) {
    std::string found_path;
    if (substitution.first.empty()) {
      found_path = CleanPath(JoinPath(substitution.second, filename));
    } else if (filename == substitution.first) {
      found_path = substitution.second;
    } else if (absl::StartsWith(filename, substitution.first + "/")) {
      found_path = CleanPath(StringReplaceFirst(filename, substitution.first,
                                                substitution.second));
    }
    std::string* stored_contents =
        found_path.empty() ? nullptr : FindOrNull(file_map_, found_path);
    if (stored_contents != nullptr) {
      VLOG(1) << "Proto file Open(" << filename << ") under ["
              << substitution.first << "->" << substitution.second << "]";
      if (!InsertIfNotPresent(file_mapping_cache_, filename, found_path)) {
        LOG(ERROR) << "Redundant/contradictory data in index or internal bug."
                   << "  \"" << filename << "\" is mapped twice, first to \""
                   << FindOrDie(*file_mapping_cache_, filename)
                   << "\" and now to \"" << found_path << "\".  Aborting "
                   << "new remapping...";
      }
      return new google::protobuf::io::ArrayInputStream(
          stored_contents->data(), stored_contents->size());
    }
  }
  std::string* stored_contents = FindOrNull(file_map_, filename);
  if (stored_contents != nullptr) {
    VLOG(1) << "Proto file Open(" << filename << ") at root";
    return new google::protobuf::io::ArrayInputStream(stored_contents->data(),
                                                      stored_contents->size());
  }
  last_error_ = "Proto file Open(" + filename + ") failed because '" +
                filename + "' not recognized by indexer";
  LOG(WARNING) << last_error_;
  return nullptr;
}

bool PreloadedProtoFileTree::Read(absl::string_view file_path,
                                  std::string* out) {
  std::unique_ptr<google::protobuf::io::ZeroCopyInputStream> in_stream(
      Open(std::string(file_path)));
  if (!in_stream) {
    return false;
  }

  const void* data = nullptr;
  int size = 0;
  while (in_stream->Next(&data, &size)) {
    out->append(static_cast<const char*>(data), size);
  }

  return true;
}

}  // namespace kythe
