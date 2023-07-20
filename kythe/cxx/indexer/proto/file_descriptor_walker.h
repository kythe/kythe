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

#ifndef KYTHE_CXX_INDEXER_PROTO_FILE_DESCRIPTOR_WALKER_H_
#define KYTHE_CXX_INDEXER_PROTO_FILE_DESCRIPTOR_WALKER_H_

#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "absl/log/log.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "kythe/cxx/common/file_vname_generator.h"
#include "kythe/cxx/common/indexing/KytheOutputStream.h"
#include "kythe/cxx/common/kythe_uri.h"
#include "kythe/cxx/common/utf8_line_index.h"
#include "kythe/cxx/indexer/proto/proto_analyzer.h"
#include "kythe/cxx/indexer/proto/proto_graph_builder.h"
#include "kythe/proto/common.pb.h"
#include "kythe/proto/storage.pb.h"
#include "kythe/proto/xref.pb.h"

namespace proto2 {
class Descriptor;
class DescriptorPool;
class EnumDescriptor;
class FileDescriptor;
}  // namespace proto2

namespace kythe {
namespace lang_proto {

// A human-readable mediator between 3/4 element "span" vectors and the proto
// compiler's SourceLocations (which contain extra info we don't always want
// to pass around).
//
// Line numbers start at 1, but column numbers start at 0. Column numbers
// correspond with byte offsets into the file except in the case of tabs,
// which advance the column number to the next multiple of 8.
struct PartialLocation {
  int start_line;
  int end_line;
  int start_column;
  int end_column;
};

// Class for walking a file descriptor and its messages, enums, etc.
// Mainly just a place to keep track of state between related methods.
class FileDescriptorWalker {
 public:
  FileDescriptorWalker(const google::protobuf::FileDescriptor* file_descriptor,
                       const google::protobuf::SourceCodeInfo& source_code_info,
                       const proto::VName& file_name,
                       const std::string& content, ProtoGraphBuilder* builder,
                       ProtoAnalyzer* analyzer)
      : file_descriptor_(file_descriptor),
        source_code_info_(&source_code_info),
        file_name_(file_name),
        content_(content),
        line_index_(kythe::UTF8LineIndex(content_)),
        builder_(builder),
        uri_(file_name_) {}

  // disallow copy and assign
  FileDescriptorWalker(const FileDescriptorWalker&) = delete;
  void operator=(const FileDescriptorWalker&) = delete;

  // Takes in a span -- as defined by SourceCodeInfo.Location.span -- and
  // converts it into a Location.
  void InitializeLocation(const std::vector<int>& span, Location* loc);

  // Adds path and span from source_code_info to location_map_ as key and value
  // respectively.
  void BuildLocationMap(
      const google::protobuf::SourceCodeInfo& source_code_info);

  // Walks through all of the imports in the descriptor and adds them to the
  // graph. Imports includes all of dependencies, weak dependencies and public
  // dependencies.
  void VisitImports();

  // Walks through the fields and declared extensions of the input
  // DescriptorProto and adds Kythe childof edges. Also looks for the type name.
  // of the field and adds a Kythe ref edge if the type name can be resolved.
  // For example, consider the field:
  // Foo bar = 2;
  // ^   ^
  // We look for the location of typename (Foo) and save that in Kythe as
  // reference location. We look for the location of the name (bar) and save in
  // Kythe as a declaration.
  // `lookup_path` is expected to point to the parent message (all of it).
  void VisitFields(const std::string& message_name,
                   const google::protobuf::Descriptor* dp,
                   std::vector<int> lookup_path);

  // Processes the declaration of an individual field.
  // `parent_name`/`parent` refer to the context this field is declared in
  // (null for top-level extensions in a package-less file).
  // `message_name`/`message` refer to the message this ticket is a part of.
  // These only differ when processing extensions.
  // `lookup_path` is expected to point to the FieldDescriptorProto being
  // processed.
  void VisitField(const std::string* parent_name, const proto::VName* parent,
                  const std::string& message_name, const proto::VName& message,
                  const google::protobuf::FieldDescriptor* field,
                  std::vector<int> lookup_path);

  // Processes the declaration of an extended field, and adds a reference
  // to the message being extended (in the "extend X {" line).
  // `parent_name`/`parent` refers to the context this field is declared in
  // (null for top-level extensions in a package-less file).
  // `lookup_path` is expected to point to the FieldDescriptorProto of the
  // extension being processed.
  void VisitExtension(const std::string* parent_name,
                      const proto::VName* parent,
                      const google::protobuf::FieldDescriptor* field,
                      std::vector<int> lookup_path);

  // Visits all the nested message types in the given DescriptorProto.
  // The nested messages are added to the codegraph.
  // `lookup_path` is used to fetch the location of declaration.
  void VisitNestedEnumTypes(const std::string& message_name,
                            const proto::VName* message,
                            const google::protobuf::Descriptor* dp,
                            std::vector<int> lookup_path);

  // Visits all the nested message types in the given DescriptorProto.
  // The nested messages are added to the codegraph.
  // `lookup_path` must point to the given DescriptorProto.
  // The lookup path is used to fetch the location of declaration.
  void VisitNestedTypes(const std::string& message_name,
                        const proto::VName* message,
                        const google::protobuf::Descriptor* dp,
                        std::vector<int> lookup_path);

  // Visits all the oneofs within a message and adds them to the codegraph.
  // `lookup_path` must point to the given DescriptorProto.
  // The lookup path is used to fetch the location of declaration; although we
  // modify the lookup path, it is left in its original state after we return.
  void VisitOneofs(const std::string& message_name, const proto::VName& message,
                   const google::protobuf::Descriptor* dp,
                   std::vector<int> lookup_path);

  // Visits all the messages and enums within a namespace. All messages and
  // enums, along with their associated fields, oneofs, and values, are added
  // to the graph.
  void VisitMessagesAndEnums(const std::string* ns_name,
                             const proto::VName* ns);

  // Visit all values in a given enum (either top-level or nested) and add
  // Kythe nodes and edges.
  // `lookup_path` must point to the enum.
  void VisitEnumValues(const google::protobuf::EnumDescriptor* dp,
                       const proto::VName* e, std::vector<int> lookup_path);

  // Method to add declarations and references for all fields.
  // We do this after all messages and enums (both top-level and nested)
  // are added to Kythe.
  void VisitAllFields(const std::string* ns_name, const proto::VName* ns);

  // Visit stubby services and input/output methods.
  void VisitRpcServices(const std::string* ns_name, const proto::VName* ns);

  // This function invokes all the Visit* functions and also adds the
  // namespace as a Kythe binding.
  void PopulateCodeGraph();

 private:
  // Converts from a proto line/column (both 0 based, and where column counts
  // bytes except that tabs move to the next multiple of 8) to a byte offset
  // from the start of the current file.  Returns -1 on error.
  int ComputeByteOffset(int line_number, int column_number) const;

  // Computes the bytes prior to the start of the element starting on
  // `entity_start_line` at `entity_start_column` that make up `comment`.
  Location LocationOfLeadingComments(const Location& entity_location,
                                     int entity_start_line,
                                     int entity_start_column,
                                     const std::string& comment) const;

  // Compute the bytes following to the end of the element starting on
  // `entity_start_line` at `entity_start_column` that make up `comment`.
  Location LocationOfTrailingComments(const Location& entity_location,
                                      int entity_start_line,
                                      int entity_start_column,
                                      const std::string& comment) const;

  // Parses a location span vector (three or four integers that protoc uses to
  // represent a location in a file) and return a sensible PartialLocation or
  // Status::INVALID_ARGUMENT if the vector cannot be properly interpreted.
  absl::StatusOr<PartialLocation> ParseLocation(
      const std::vector<int>& span) const;

  std::optional<proto::VName> VNameForFieldType(
      const google::protobuf::FieldDescriptor* field);

  /// \brief Attach marked source (if not None) to `vname`.
  void AttachMarkedSource(const proto::VName& vname,
                          const std::optional<MarkedSource>& code);

  const google::protobuf::FileDescriptor* file_descriptor_;
  const google::protobuf::SourceCodeInfo* source_code_info_;
  const proto::VName file_name_;
  const std::string content_;
  const kythe::UTF8LineIndex line_index_;
  ProtoGraphBuilder* builder_;
  URI uri_;
  std::map<std::vector<int>, std::vector<int> > location_map_;
  std::map<std::vector<int>, google::protobuf::SourceCodeInfo::Location>
      path_location_map_;

  // Set of messages for which their fields are already visited.
  // There are two functions from which 'VisitFields' gets called;
  // 'VisitAllFields' and 'VisitNestedTypes'. This causes analyzer to create
  // duplicate entries for some nodes. This set helps us avoid processing
  // fields more than once.
  std::set<std::string> visited_messages_;

  // Adds leading and trailing comments for the element specified by ticket and
  // path. `v_name` is the name of the element in question; `path` is used
  // to look up the SourceCodeInfo::Location and the retrieve comment locations.
  void AddComments(const proto::VName& v_name, const std::vector<int>& path);

  // This recursively visits nested fields for VisitAllFields, with the current
  // parent scope specified by name_prefix, message-descriptor 'dp' and
  // lookup_path for source information lookup.
  void VisitNestedFields(const std::string& name_prefix,
                         const google::protobuf::Descriptor* dp,
                         std::vector<int> lookup_path);

  // Checks for generated proto info
  void VisitGeneratedProtoInfo();
};

}  // namespace lang_proto
}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_PROTO_FILE_DESCRIPTOR_WALKER_H_
