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

#include "absl/container/flat_hash_map.h"
#include "kythe/cxx/common/file_vname_generator.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/cxx/common/protobuf_metadata_file.h"
#include "kythe/cxx/indexer/proto/proto_analyzer.h"
#include "kythe/cxx/indexer/proto/search_path.h"
#include "kythe/cxx/indexer/proto/source_tree.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {

std::string IndexProtoCompilationUnit(
    const proto::CompilationUnit& unit,
    const std::vector<proto::FileData>& files,
    const kythe::MetadataSupports& meta_supports, KytheOutputStream* output) {
  KytheGraphRecorder recorder(output);

  std::vector<std::string> unprocessed_args;
  std::vector<std::pair<std::string, std::string>> path_substitutions;
  ::kythe::lang_proto::ParsePathSubstitutions(
      unit.argument(), &path_substitutions, &unprocessed_args);
  if (path_substitutions.empty() && !unit.working_directory().empty()) {
    path_substitutions.push_back({"", CleanPath(unit.working_directory())});
  }

  absl::flat_hash_map<std::string, std::string> file_substitution_cache;
  PreloadedProtoFileTree file_reader(&path_substitutions,
                                     &file_substitution_cache);
  google::protobuf::compiler::SourceTreeDescriptorDatabase descriptor_db(
      &file_reader);
  for (const auto& file_data : files) {
    file_reader.AddFile(file_data.info().path(), file_data.content());
  }
  lang_proto::ProtoAnalyzer analyzer(&unit, &descriptor_db, &recorder,
                                     &file_substitution_cache);
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
    if (!analyzer.Parse(file_path, file_contents, meta_supports)) {
      errors += "\n Analyzer failed on " + file_path;
    }
  }
  if (!errors.empty()) {
    return "Errors during indexing:" + errors;
  }

  return "";
}

}  // namespace kythe
