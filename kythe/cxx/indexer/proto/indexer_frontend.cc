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
#include "kythe/cxx/common/file_vname_generator.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/cxx/indexer/proto/proto_analyzer.h"
#include "kythe/cxx/indexer/proto/source_tree.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {

namespace {
void AddPathSubstitutions(
    absl::string_view path_argument,
    std::vector<std::pair<std::string, std::string>>* substitutions) {
  std::vector<std::string> parts =
      absl::StrSplit(path_argument, ':', absl::SkipEmpty());
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
    std::vector<std::pair<std::string, std::string>>* substitutions) {
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
                                      const std::vector<proto::FileData>& files,
                                      KytheOutputStream* output) {
  FileVNameGenerator file_vnames;
  KytheGraphRecorder recorder(output);
  std::vector<std::pair<std::string, std::string>> path_substitutions;
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
