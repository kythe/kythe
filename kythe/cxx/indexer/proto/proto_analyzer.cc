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

#include "kythe/cxx/indexer/proto/proto_analyzer.h"

#include "absl/container/flat_hash_map.h"
#include "absl/log/log.h"
#include "absl/strings/string_view.h"
#include "kythe/cxx/indexer/proto/file_descriptor_walker.h"

namespace kythe {
namespace lang_proto {

using ::google::protobuf::FileDescriptorProto;
using ::kythe::proto::VName;

// TODO: it seems very likely that a lot of the path-mangling
// logic can be removed, though it doesn't seem to cause any harm...

// It is apparently preferred to use the ProtoFileParser api rather than
// google::protobuf::compiler::parser. We also use the DescriptorPool to fetch
// filenames of resolved type vnames.
ProtoAnalyzer::ProtoAnalyzer(
    const proto::CompilationUnit* unit,
    google::protobuf::DescriptorDatabase* descriptor_db,
    KytheGraphRecorder* recorder,
    absl::flat_hash_map<std::string, std::string>* path_substitution_cache)
    : unit_(unit),
      recorder_(recorder),
      path_substitution_cache_(path_substitution_cache),
      descriptor_db_(descriptor_db) {}

bool ProtoAnalyzer::AnalyzeFile(const std::string& rel_path,
                                const VName& v_name,
                                const std::string& content) {
  google::protobuf::DescriptorPool pool(descriptor_db_);
  ProtoGraphBuilder builder(recorder_, [&](const std::string& path) {
    return VNameFromRelPath(path);
  });

  // We keep track of all visited files, effectively performing per-replica
  // claiming.  Formerly this helped avoid issues with cyclic dependencies, but
  // now only the reused proto2 infrastructure descends into dependencies and
  // thus only it is exposed to cyclic dependency risks.
  if (!visited_files_.insert(rel_path).second) {
    return true;
  }

  builder.SetText(v_name, content);

  // Note: It would appear to be cleaner to have these calls within
  // FileDescriptorWalker.  However, for files which fail to have a valid
  // FileDescriptor (which actually happens just processing the files in
  // devtools/grok/proto/, it turns out), we at least get the partial info
  // of having the file in our index and acknowledging the lexer results.
  builder.AddNode(v_name, NodeKindID::kFile);

  // TODO: If FileDescriptor surfaced the source code info, then we
  // wouldn't need to look up the proto as well.
  const google::protobuf::FileDescriptor* descriptor =
      pool.FindFileByName(rel_path);
  google::protobuf::FileDescriptorProto descriptor_proto;
  if ((descriptor == nullptr) ||
      !descriptor_db_->FindFileByName(rel_path, &descriptor_proto)) {
    // TODO: We should be associating any such "diagnostic" messages
    // with the file, so that we are aware of problems with files without
    // reanalyzing them.
    LOG(ERROR) << (descriptor == nullptr ? "File not found: "
                                         : "Error parsing: ")
               << rel_path;
    // Undo the cache recording of visitation we performed at function entry in
    // case another context can successfully read this later.
    visited_files_.erase(rel_path);
    return false;
  }

  FileDescriptorWalker walker(descriptor, descriptor_proto.source_code_info(),
                              v_name, content, &builder, this);
  walker.PopulateCodeGraph();
  return true;
}

bool ProtoAnalyzer::Parse(const std::string& proto_file,
                          const std::string& content) {
  DLOG(LEVEL(-1)) << "FILE : " << proto_file << std::endl;
  return AnalyzeFile(proto_file, VNameFromFullPath(proto_file), content);
}

VName ProtoAnalyzer::VNameFromRelPath(
    const std::string& simplified_path) const {
  const std::string* full_path = &simplified_path;
  if (auto iter = path_substitution_cache_->find(simplified_path);
      iter != path_substitution_cache_->end()) {
    full_path = &iter->second;
  }
  return VNameFromFullPath(*full_path);
}

VName ProtoAnalyzer::VNameFromFullPath(const std::string& path) const {
  for (const auto& input : unit_->required_input()) {
    if (input.info().path() == path) {
      return input.v_name();
    }
  }
  return VName{};
}

}  // namespace lang_proto
}  // namespace kythe
