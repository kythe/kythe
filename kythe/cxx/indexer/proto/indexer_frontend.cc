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

#include "kythe/cxx/indexer/proto/indexer_frontend.h"

#include "absl/container/node_hash_map.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "google/protobuf/stubs/map_util.h"
#include "kythe/cxx/common/file_vname_generator.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/cxx/indexer/proto/proto_analyzer.h"
#include "kythe/proto/analysis.pb.h"

using absl::StrSplit;
using std::pair;
using std::vector;

namespace kythe {

using google::protobuf::FindOrDie;
using google::protobuf::FindOrNull;
using google::protobuf::InsertIfNotPresent;

namespace {

// TODO(justbuchanan): why isn't there a replace_all=false version of
// StrReplace() in open-source abseil?
/// Finds the first occurrence of @oldsub in @s and replaces it with @newsub. If
/// @oldsub is not present, just returns @s.
std::string StringReplaceFirst(absl::string_view s, absl::string_view oldsub,
                               absl::string_view newsub) {
  if (oldsub.empty()) {
    return std::string(s);
  }

  std::string result;

  absl::string_view::size_type start_pos = 0;
  absl::string_view::size_type pos = s.find(oldsub, start_pos);
  if (pos != absl::string_view::npos) {
    result.append(s.data() + start_pos, pos - start_pos);
    result.append(newsub.data(), newsub.length());
    start_pos = pos + oldsub.length();
  }
  result.append(s.data() + start_pos, s.length() - start_pos);

  return result;
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
  for (const pair<std::string, std::string>& substitution : *substitutions_) {
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
  auto* in_stream = Open(std::string(file_path));
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

namespace {
void AddPathSubstitutions(
    absl::string_view path_argument,
    vector<pair<std::string, std::string>>* substitutions) {
  vector<std::string> parts = StrSplit(path_argument, ':', absl::SkipEmpty());
  for (const std::string& path_or_substitution : parts) {
    std::string::size_type equals_pos = path_or_substitution.find_first_of('=');
    if (equals_pos == std::string::npos) {
      substitutions->push_back(
          std::make_pair("", CleanPath(path_or_substitution)));
    } else {
      substitutions->push_back(std::make_pair(
          CleanPath(path_or_substitution.substr(0, equals_pos)),
          CleanPath(path_or_substitution.substr(equals_pos + 1))));
    }
  }
}

void ParsePathSubstitutions(
    const proto::CompilationUnit& unit,
    vector<pair<std::string, std::string>>* substitutions) {
  bool have_paths = false;
  bool expecting_path_arg = false;
  for (const std::string& argument : unit.argument()) {
    if (expecting_path_arg) {
      expecting_path_arg = false;
      AddPathSubstitutions(argument, substitutions);
      have_paths = true;
    } else if (argument == "-I" || argument == "--proto_path") {
      expecting_path_arg = true;
    } else {
      absl::string_view argument_value = argument;
      if (absl::ConsumePrefix(&argument_value, "-I")) {
        AddPathSubstitutions(argument_value, substitutions);
        have_paths = true;
      } else if (absl::ConsumePrefix(&argument_value, "--proto_path=")) {
        AddPathSubstitutions(argument_value, substitutions);
        have_paths = true;
      }
    }
  }
  if (!have_paths && !unit.working_directory().empty()) {
    substitutions->push_back(
        std::make_pair("", CleanPath(unit.working_directory())));
  }
}
}  // anonymous namespace

std::string IndexProtoCompilationUnit(const proto::CompilationUnit& unit,
                                      const vector<proto::FileData>& files,
                                      KytheOutputStream* output) {
  FileVNameGenerator file_vnames;
  KytheGraphRecorder recorder(output);
  vector<pair<std::string, std::string>> path_substitutions;
  absl::node_hash_map<std::string, std::string> file_substitution_cache;
  ParsePathSubstitutions(unit, &path_substitutions);
  PreloadedProtoFileTree file_reader(&path_substitutions,
                                     &file_substitution_cache);
  google::protobuf::compiler::SourceTreeDescriptorDatabase descriptor_db(
      &file_reader);
  for (const auto& file_data : files) {
    file_reader.AddFile(file_data.info().path(), file_data.content());
  }
  lang_proto::ProtoAnalyzer analyzer(&unit, &descriptor_db, &file_vnames,
                                     &recorder, &file_substitution_cache);
  if (unit.source_file().empty()) {
    return "Error: no source_files in CompilationUnit.";
  }
  std::string errors;
  for (const std::string& file_path : unit.source_file()) {
    if (file_path.empty()) {
      errors += "\n empty source_file.";
      continue;
    }
    std::string file_contents;
    if (!file_reader.Read(file_path, &file_contents)) {
      errors += "\n source_file " + file_path + " not in FileData.";
      continue;
    }
    if (!analyzer.Parse(file_path, file_contents)) {
      errors += "\n Analyzer failed on " + file_path;
    }
  }
  if (!errors.empty()) {
    return "Errors during indexing:" + errors;
  }

  return "";
}

}  // namespace kythe
