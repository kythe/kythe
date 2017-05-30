/*
 * Copyright 2016 Google Inc. All rights reserved.
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

#include "kythe/cxx/common/protobuf_metadata_file.h"

#include <sstream>

#include "glog/logging.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

namespace kythe {

proto::VName ProtobufMetadataSupport::VNameForAnnotation(
    const proto::VName &context_vname,
    const google::protobuf::GeneratedCodeInfo::Annotation &annotation) {
  proto::VName out;
  if (!vname_lookup_(annotation.source_file(), &out)) {
    out.set_corpus(context_vname.corpus());
  }
  out.set_path(annotation.source_file());
  return VNameForProtoPath(out, annotation.path());
}

std::unique_ptr<kythe::MetadataFile> ProtobufMetadataSupport::ParseFile(
    const std::string &raw_filename, const std::string &filename,
    const llvm::MemoryBuffer *buffer) {
  llvm::StringRef file_ref(filename);
  if (!file_ref.endswith(".pb.h.meta") && !file_ref.endswith(".proto.h.meta")) {
    return nullptr;
  }
  proto::VName context_vname;
  if (!vname_lookup_(raw_filename, &context_vname)) {
    LOG(WARNING) << "Failed getting VName for metadata: " << raw_filename;
  }
  google::protobuf::GeneratedCodeInfo info;
  if (!info.ParseFromArray(buffer->getBufferStart(), buffer->getBufferSize())) {
    LOG(WARNING) << "Failed ParseFromArray: " << raw_filename;
    return nullptr;
  }
  std::vector<MetadataFile::Rule> rules;
  for (const auto &annotation : info.annotation()) {
    MetadataFile::Rule rule;
    rule.begin = annotation.begin();
    rule.end = annotation.end();
    rule.vname = VNameForAnnotation(context_vname, annotation);
    rule.edge_in = "/kythe/edge/defines/binding";
    rule.edge_out = "/kythe/edge/generates";
    rule.reverse_edge = true;
    rules.push_back(rule);
  }
  return MetadataFile::LoadFromRules(rules.begin(), rules.end());
}
}  // namespace kythe
