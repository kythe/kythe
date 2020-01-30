/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

#include "absl/strings/match.h"
#include "glog/logging.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/cxx/common/schema/edges.h"
#include "kythe/cxx/common/vname_ordering.h"

namespace kythe {

proto::VName ProtobufMetadataSupport::VNameForAnnotation(
    const proto::VName& context_vname,
    const google::protobuf::GeneratedCodeInfo::Annotation& annotation) {
  proto::VName out;
  if (!vname_lookup_(annotation.source_file(), &out)) {
    out.set_corpus(context_vname.corpus());
  }
  out.set_path(annotation.source_file());
  return VNameForProtoPath(out, annotation.path());
}

std::unique_ptr<kythe::MetadataFile> ProtobufMetadataSupport::ParseFile(
    const std::string& raw_filename, const std::string& filename,
    absl::string_view buffer) {
  absl::string_view file_ref(filename);
  if (!(absl::EndsWith(filename, ".pb.h.meta") ||
        absl::EndsWith(filename, ".pb.h") ||
        absl::EndsWith(filename, ".proto.h.meta") ||
        absl::EndsWith(filename, ".proto.h") ||
        absl::EndsWith(filename, ".stubby.h"))) {
    return nullptr;
  }
  proto::VName context_vname;
  if (!vname_lookup_(raw_filename, &context_vname)) {
    LOG(WARNING) << "Failed getting VName for metadata: " << raw_filename;
  }
  google::protobuf::GeneratedCodeInfo info;
  if (!info.ParseFromArray(buffer.data(), buffer.size())) {
    LOG(WARNING) << "Failed ParseFromArray: " << raw_filename;
    return nullptr;
  }

  std::set<proto::VName, kythe::VNameLess> vnames;
  std::vector<MetadataFile::Rule> rules;
  for (const auto& annotation : info.annotation()) {
    MetadataFile::Rule rule;
    rule.whole_file = false;
    rule.begin = annotation.begin();
    rule.end = annotation.end();
    rule.vname = VNameForAnnotation(context_vname, annotation);
    rule.edge_in = kythe::common::schema::kDefinesBinding;
    rule.edge_out = kythe::common::schema::kGenerates;
    rule.reverse_edge = true;
    rule.generate_anchor = false;
    rule.anchor_begin = 0;
    rule.anchor_end = 0;
    rules.push_back(rule);
    if (!rule.vname.path().empty()) {
      vnames.insert(rule.vname);
    }
  }

  // Create file-scoped rules for all encountered vnames.
  for (const auto& vname : vnames) {
    MetadataFile::Rule rule;
    rule.whole_file = true;
    rule.vname = vname;
    rule.vname.set_signature("");
    rule.vname.set_language("");
    rule.edge_out = kythe::common::schema::kGenerates;
    rule.reverse_edge = true;
    rule.generate_anchor = false;
    rules.push_back(rule);
  }

  return MetadataFile::LoadFromRules(raw_filename, rules.begin(), rules.end());
}
}  // namespace kythe
