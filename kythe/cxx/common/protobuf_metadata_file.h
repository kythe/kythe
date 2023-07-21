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

#ifndef KYTHE_CXX_COMMON_PROTOBUF_METADATA_FILE_H_
#define KYTHE_CXX_COMMON_PROTOBUF_METADATA_FILE_H_

#include <memory>
#include <sstream>

#include "absl/log/log.h"
#include "google/protobuf/descriptor.pb.h"
#include "kythe/cxx/common/file_vname_generator.h"
#include "kythe/cxx/common/kythe_metadata_file.h"

namespace kythe {

/// \brief Generates a VName for the given proto path.
/// \param file_vname the VName for the proto file. Its signature and language
/// will be ignored.
/// \param path the proto AST path.
template <typename Path>
proto::VName VNameForProtoPath(const proto::VName& file_vname,
                               const Path& path) {
  proto::VName out(file_vname);
  std::string signature;
  std::stringstream sig(signature);
  bool first_node = true;
  for (const auto& node : path) {
    sig << (first_node ? "" : ".") << node;
    first_node = false;
  }
  out.set_signature(sig.str());
  out.set_language("protobuf");
  return out;
}

/// Provides metadata support for proto2.GeneratedCodeInfo metadata.
class ProtobufMetadataSupport : public MetadataSupport {
 public:
  /// Parse and return a metadata file. Will return null if `filename` does not
  /// end in .pb.h.meta or .proto.h.meta. If this check passes and `buffer` does
  /// not parse into a `proto2.GeneratedCodeInfo` message, returns null and logs
  /// a warning.
  std::unique_ptr<kythe::MetadataFile> ParseFile(
      const std::string& raw_filename, const std::string& filename,
      absl::string_view buffer, absl::string_view target_buffer) override;

  void UseVNameLookup(VNameLookup lookup) override { vname_lookup_ = lookup; }

  void SetAliasesAsWrites(bool set_aliases_as_writes) {
    set_aliases_as_writes_ = set_aliases_as_writes;
  }

 private:
  /// Returns the VName for the node that should be used between
  /// generated semantic objects and the objects that generate them.
  /// \param context_vname the VName to use when a .proto's VName can't be
  /// identified.
  proto::VName VNameForAnnotation(
      const proto::VName& context_vname,
      const google::protobuf::GeneratedCodeInfo::Annotation& annotation);

  /// Used to build VNames for proto objects.
  VNameLookup vname_lookup_ = [](const std::string& path, proto::VName* out) {
    return false;
  };

  /// Use write semantics for aliases.
  bool set_aliases_as_writes_ = false;
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_PROTOBUF_METADATA_FILE_H_
