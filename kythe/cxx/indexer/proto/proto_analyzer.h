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

#ifndef KYTHE_CXX_INDEXER_PROTO_PROTO_ANALYZER_H_
#define KYTHE_CXX_INDEXER_PROTO_PROTO_ANALYZER_H_

#include <memory>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/container/node_hash_set.h"
#include "absl/log/log.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor_database.h"
#include "kythe/cxx/common/file_vname_generator.h"
#include "kythe/cxx/common/kythe_uri.h"
#include "kythe/cxx/indexer/proto/proto_graph_builder.h"
#include "kythe/proto/analysis.pb.h"
#include "kythe/proto/common.pb.h"
#include "kythe/proto/storage.pb.h"
#include "kythe/proto/xref.pb.h"

namespace kythe {
namespace lang_proto {

// Class to parse proto files, analyze them, create a proto graph builder and
// output the serialized IndexArtifacts in the form of an SSTable.
class ProtoAnalyzer {
 public:
  // Constructs a new analyzer with the given path and graph recorder. The
  // latter must remain alive/valid the entire duration of the ProtoAnalyzer
  // instance, but ownership is not transferred.
  ProtoAnalyzer(
      const proto::CompilationUnit* unit,
      google::protobuf::DescriptorDatabase* descriptor_db,
      KytheGraphRecorder* recorder,
      absl::flat_hash_map<std::string, std::string>* path_substitution_cache);

  // disallow copy and assign
  ProtoAnalyzer(const ProtoAnalyzer&) = delete;
  void operator=(const ProtoAnalyzer&) = delete;

  // A wrapper for AnalyzeFile that generates the VName and relativizes
  // the proto file path.
  bool Parse(const std::string& proto_file, const std::string& content);

  // Given a string which contains a proto file, analyze it and record the
  // results.
  bool AnalyzeFile(const std::string& rel_path, const proto::VName& v_name,
                   const std::string& content);

  // Returns a VName for the input 'simplified_path' joined with any prefix
  // (for example, a bazel-out/ subdirectory) needed to properly and fully
  // reference the true storage location.  If there is no such prefix path,
  // then the input path is simply returned.
  proto::VName VNameFromRelPath(const std::string& simplified_path) const;

 private:
  absl::node_hash_set<std::string> visited_files_;

  // Returns the VName associated with `path` in unit_'s required inputs, or
  // an empty vname if none is found.
  proto::VName VNameFromFullPath(const std::string& path) const;

  // Compilation unit to be analyzed.
  const proto::CompilationUnit* unit_;

  // Where we output Kythe artifacts.
  KytheGraphRecorder* recorder_;

  // Maps files to paths which include their prefix directories, the most
  // common being bazel-out/ derivatives.
  absl::flat_hash_map<std::string, std::string>* path_substitution_cache_;

  // Gives us properly linked together descriptors for proto files and their
  // contents.
  google::protobuf::DescriptorDatabase* descriptor_db_;
};

}  // namespace lang_proto
}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_PROTO_PROTO_ANALYZER_H_
