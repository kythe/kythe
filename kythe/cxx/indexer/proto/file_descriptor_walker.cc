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

#include "kythe/cxx/indexer/proto/file_descriptor_walker.h"

#include <optional>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/repeated_field.h"
#include "kythe/cxx/common/kythe_metadata_file.h"
#include "kythe/cxx/common/schema/edges.h"
#include "kythe/cxx/indexer/proto/marked_source.h"
#include "kythe/cxx/indexer/proto/offset_util.h"
#include "kythe/cxx/indexer/proto/proto_graph_builder.h"
#include "kythe/proto/generated_message_info.pb.h"
#include "re2/re2.h"

namespace kythe {
namespace lang_proto {
namespace {

using ::google::protobuf::Descriptor;
using ::google::protobuf::DescriptorProto;
using ::google::protobuf::EnumDescriptor;
using ::google::protobuf::EnumDescriptorProto;
using ::google::protobuf::EnumValueDescriptor;
using ::google::protobuf::EnumValueDescriptorProto;
using ::google::protobuf::FieldDescriptor;
using ::google::protobuf::FieldDescriptorProto;
using ::google::protobuf::FileDescriptorProto;
using ::google::protobuf::MethodDescriptor;
using ::google::protobuf::MethodDescriptorProto;
using ::google::protobuf::OneofDescriptor;
using ::google::protobuf::ServiceDescriptor;
using ::google::protobuf::ServiceDescriptorProto;
using ::google::protobuf::SourceCodeInfo;
using ::kythe::proto::VName;

// Pushes a value onto a proto location lookup path, and automatically
// removes it when destroyed.  See the documentation for
// proto2.Descriptor.SourceCodeInfo.Location.path for more information
// on how these paths work.
class ScopedLookup {
 public:
  // Does not take ownership of lookup_path; caller must ensure it
  // stays around until this ScopedLookup is destroyed.
  explicit ScopedLookup(std::vector<int>* lookup_path, int component)
      : lookup_path_(lookup_path), component_(component) {
    lookup_path->push_back(component);
  }
  ~ScopedLookup() {
    CHECK(!lookup_path_->empty());
    CHECK_EQ(component_, lookup_path_->back());
    lookup_path_->pop_back();
  }

 private:
  std::vector<int>* lookup_path_;
  const int component_;
};

std::optional<absl::string_view> TypeName(const EnumDescriptor& desc) {
  return desc.name();
}

std::optional<absl::string_view> TypeName(const Descriptor& desc) {
  return desc.name();
}

std::optional<absl::string_view> TypeName(const FieldDescriptor& field) {
  if (field.is_map()) {
    return std::nullopt;
  }
  if (const EnumDescriptor* desc = field.enum_type()) {
    return TypeName(*desc);
  }
  if (const Descriptor* desc = field.message_type()) {
    return TypeName(*desc);
  }
  return std::nullopt;
}

template <typename DescriptorType>
void TruncateLocationToTypeName(Location& location,
                                const DescriptorType& desc) {
  std::optional<absl::string_view> type_name = TypeName(desc);
  if (!type_name.has_value() || location.end <= location.begin ||
      (location.end - location.begin) <= type_name->size()) {
    return;
  }
  location.begin = (location.end - type_name->size());
}

}  // namespace

int FileDescriptorWalker::ComputeByteOffset(int line_number,
                                            int column_number) const {
  int byte_offset_of_start_of_line =
      line_index_.ComputeByteOffset(line_number, 0);
  absl::string_view line_text = line_index_.GetLine(line_number);
  int byte_offset_into_line =
      ByteOffsetOfTabularColumn(line_text, column_number);
  if (byte_offset_into_line < 0) {
    return byte_offset_into_line;
  }
  return byte_offset_of_start_of_line + byte_offset_into_line;
}

Location FileDescriptorWalker::LocationOfLeadingComments(
    const Location& entity_location, int entity_start_line,
    int entity_start_column, const std::string& comments) const {
  int line_offset_of_entity = ByteOffsetOfTabularColumn(
      line_index_.GetLine(entity_start_line), entity_start_column);
  if (line_offset_of_entity < 0) {
    return entity_location;
  }
  Location comment_location;
  comment_location.file = entity_location.file;
  comment_location.begin = entity_location.begin - line_offset_of_entity;
  comment_location.end = entity_location.begin - line_offset_of_entity - 1;
  int next_line_number = entity_start_line - 1;
  absl::string_view bottom_line = line_index_.GetLine(next_line_number);
  while (RE2::FullMatch(bottom_line, R"((\s*\*/?\s*)|(\s*//\n))")) {
    comment_location.begin -= bottom_line.size();
    --next_line_number;
    bottom_line = line_index_.GetLine(next_line_number);
  }
  std::vector<std::string> comment_lines = absl::StrSplit(comments, '\n');
  while (!comment_lines.empty() && comment_lines.back().empty()) {
    comment_lines.pop_back();
  }
  while (!comment_lines.empty()) {
    const std::string& comment_line = comment_lines.back();
    absl::string_view actual_line = line_index_.GetLine(next_line_number);
    std::string comment_re =
        absl::StrCat(R"(\s*(?://|/?\*\s*))", RE2::QuoteMeta(comment_line),
                     R"(\s*(?:\*/)?\s*)");
    if (!RE2::FullMatch(actual_line, comment_re)) {
      LOG(ERROR) << "Leading comment line mismatch: [" << comment_line
                 << "] vs. [" << actual_line << "]"
                 << "(line " << next_line_number << ")";
      return comment_location;
    }
    comment_location.begin -= actual_line.size();
    --next_line_number;
    comment_lines.pop_back();
  }
  return comment_location;
}

Location FileDescriptorWalker::LocationOfTrailingComments(
    const Location& entity_location, int entity_start_line,
    int entity_start_column, const std::string& comments) const {
  Location comment_location;
  comment_location.file = entity_location.file;
  std::vector<std::string> comment_lines = absl::StrSplit(comments, '\n');
  while (!comment_lines.empty() && comment_lines.back().empty()) {
    comment_lines.pop_back();
  }
  if (comment_lines.empty()) {
    LOG(ERROR) << "Trailing comment listed as present but was empty.";
    return entity_location;
  }
  std::string top_comment_line_re = absl::StrCat(
      R"(\s*(?:/\*|//)\s*)", RE2::QuoteMeta(comment_lines.front()));
  int line_number = entity_start_line;
  for (; line_number <= line_index_.line_count(); ++line_number) {
    absl::string_view entity_line = line_index_.GetLine(line_number);
    absl::string_view comment_start;
    if (RE2::PartialMatch(entity_line, R"((\s*(?:/\*|//)))", &comment_start)) {
      comment_location.begin = line_index_.ComputeByteOffset(line_number, 0) +
                               (comment_start.data() - entity_line.data());
      comment_location.end =
          line_index_.ComputeByteOffset(line_number + 1, 0) - 1;
      if (RE2::PartialMatch(entity_line, top_comment_line_re)) {
        comment_lines.erase(comment_lines.begin());
      }
      break;
    }
  }
  if (line_number > line_index_.line_count()) {
    LOG(ERROR) << "Never found trailing comment \"" << comments << "\"";
    return entity_location;
  }
  ++line_number;
  for (const std::string& comment_line : comment_lines) {
    absl::string_view actual_line = line_index_.GetLine(line_number);
    std::string comment_re =
        absl::StrCat(R"(\s*(?://|/?\*\s*))", RE2::QuoteMeta(comment_line),
                     R"(\s*(?:\*/)?\s*)");
    if (!RE2::FullMatch(actual_line, comment_re)) {
      LOG(ERROR) << "Trailing comment line mismatch: [" << comment_line
                 << "] vs. [" << actual_line << "]"
                 << "(line " << line_number << ")";
      return comment_location;
    }
    comment_location.end += actual_line.size();
    ++line_number;
  }

  absl::string_view bottom_line = line_index_.GetLine(line_number);
  while (RE2::FullMatch(bottom_line, R"(\s*\*/?\s*)")) {
    comment_location.end += bottom_line.size();
    ++line_number;
    bottom_line = line_index_.GetLine(line_number);
  }
  return comment_location;
}

absl::StatusOr<PartialLocation> FileDescriptorWalker::ParseLocation(
    const std::vector<int>& span) const {
  PartialLocation location;
  if (span.size() == 4) {
    location.start_line = span[0] + 1;
    location.end_line = span[2] + 1;
    location.start_column = span[1];
    location.end_column = span[3];
  } else if (span.size() == 3) {
    location.start_line = span[0] + 1;
    location.end_line = span[0] + 1;
    location.start_column = span[1];
    location.end_column = span[2];
  } else {
    return absl::UnknownError("");
  }
  return location;
}

void FileDescriptorWalker::InitializeLocation(const std::vector<int>& span,
                                              Location* loc) {
  loc->file = file_name_;
  absl::StatusOr<PartialLocation> possible_location = ParseLocation(span);
  if (possible_location.ok()) {
    PartialLocation partial_location = *possible_location;
    loc->begin = ComputeByteOffset(partial_location.start_line,
                                   partial_location.start_column);
    loc->end = ComputeByteOffset(partial_location.end_line,
                                 partial_location.end_column);
  } else {
    // Some error in the span, create a dummy location for now
    // Happens in case of proto1 files
    LOG(ERROR) << "Unexpected location vector [" << absl::StrJoin(span, ":")
               << "] while walking " << file_name_.path();
    loc->begin = 0;
    loc->end = 0;
  }
}

void FileDescriptorWalker::BuildLocationMap(
    const SourceCodeInfo& source_code_info) {
  for (int i = 0; i < source_code_info.location_size(); i++) {
    const SourceCodeInfo::Location& location = source_code_info.location(i);
    std::vector<int> path(location.path().begin(), location.path().end());
    std::vector<int> span(location.span().begin(), location.span().end());
    location_map_[path] = span;
    path_location_map_[path] = location;
  }
}

void FileDescriptorWalker::VisitImports() {
  {
    // Direct dependencies, from `import "foo.proto"` statements.
    std::vector<int> path = {FileDescriptorProto::kDependencyFieldNumber};
    for (int i = 0; i < file_descriptor_->dependency_count(); i++) {
      ScopedLookup import_lookup(&path, i);
      Location location;
      InitializeLocation(location_map_[path], &location);
      builder_->AddImport(file_descriptor_->dependency(i)->name(), location);
    }
  }
  {
    // Weak dependencies, from `import weak "foo.proto"` statements.
    std::vector<int> path = {FileDescriptorProto::kWeakDependencyFieldNumber};
    for (int i = 0; i < file_descriptor_->weak_dependency_count(); i++) {
      ScopedLookup import_lookup(&path, i);
      Location location;
      InitializeLocation(location_map_[path], &location);
      builder_->AddImport(file_descriptor_->weak_dependency(i)->name(),
                          location);
    }
  }
  {
    // Public dependencies, from `import public "foo.proto"` statements
    std::vector<int> path = {FileDescriptorProto::kPublicDependencyFieldNumber};
    for (int i = 0; i < file_descriptor_->public_dependency_count(); i++) {
      ScopedLookup import_lookup(&path, i);
      Location location;
      InitializeLocation(location_map_[path], &location);
      builder_->AddImport(file_descriptor_->public_dependency(i)->name(),
                          location);
    }
  }
}

namespace {
std::string SignAnnotation(
    const google::protobuf::GeneratedCodeInfo::Annotation& annotation) {
  return absl::StrJoin(annotation.path(), ".");
}

VName VNameForAnnotation(
    const VName& context_vname,
    const google::protobuf::GeneratedCodeInfo::Annotation& annotation) {
  VName out;
  out.set_corpus(context_vname.corpus());
  out.set_path(annotation.source_file());
  out.set_signature(SignAnnotation(annotation));
  out.set_language(kLanguageName);
  return out;
}
}  // anonymous namespace

void FileDescriptorWalker::VisitGeneratedProtoInfo() {
  if (!file_descriptor_->options().HasExtension(proto::generated_proto_info)) {
    return;
  }
  const google::protobuf::GeneratedCodeInfo& info =
      file_descriptor_->options()
          .GetExtension(proto::generated_proto_info)
          .generated_code_info();

  std::vector<MetadataFile::Rule> rules;
  int file_rule = -1;
  for (const auto& annotation : info.annotation()) {
    MetadataFile::Rule rule{};
    rule.whole_file = false;
    rule.begin = annotation.begin();
    rule.end = annotation.end();
    rule.vname = VNameForAnnotation(file_name_, annotation);
    rule.edge_in = kythe::common::schema::kDefinesBinding;
    rule.edge_out = kythe::common::schema::kGenerates;
    rule.reverse_edge = true;
    rule.generate_anchor = false;
    rule.anchor_begin = 0;
    rule.anchor_end = 0;
    rules.push_back(rule);
    if (!rule.vname.path().empty()) {
      if (file_rule < 0 || rule.begin > rules[file_rule].begin) {
        file_rule = rules.size() - 1;
      }
    }
  }

  // Add a file-scoped rule for the last encountered vname.
  if (file_rule >= 0) {
    MetadataFile::Rule rule{};
    rule.whole_file = true;
    rule.vname = rules[file_rule].vname;
    rule.vname.set_signature("");
    rule.vname.set_language("");
    rule.edge_out = kythe::common::schema::kGenerates;
    rule.reverse_edge = true;
    rule.generate_anchor = false;
    rules.push_back(rule);
  }

  auto meta = MetadataFile::LoadFromRules(file_name_.path(), rules.begin(),
                                          rules.end());
  builder_->SetMetadata(std::move(meta));
  builder_->MaybeAddMetadataFileRules(file_name_);
}

namespace {
std::optional<proto::VName> VNameForBuiltinType(FieldDescriptor::Type type) {
  // TODO(zrlk): Emit builtins.
  return std::nullopt;
}
}  // anonymous namespace

std::optional<proto::VName> FileDescriptorWalker::VNameForFieldType(
    const FieldDescriptor* field_proto) {
  if (field_proto->is_map()) {
    // Maps are technically TYPE_MESSAGE, but don't have a useful VName.
    return std::nullopt;
  }
  if (field_proto->type() == FieldDescriptor::TYPE_MESSAGE ||
      field_proto->type() == FieldDescriptor::TYPE_GROUP) {
    return builder_->VNameForDescriptor(field_proto->message_type());
  } else if (field_proto->type() == FieldDescriptor::TYPE_ENUM) {
    return builder_->VNameForDescriptor(field_proto->enum_type());
  } else {
    return VNameForBuiltinType(field_proto->type());
  }
}

void FileDescriptorWalker::AttachMarkedSource(
    const proto::VName& vname, const std::optional<MarkedSource>& code) {
  if (code) {
    builder_->AddCodeFact(vname, *code);
  }
}

void FileDescriptorWalker::VisitField(const std::string* parent_name,
                                      const VName* parent,
                                      const std::string& message_name,
                                      const VName& message,
                                      const FieldDescriptor* field,
                                      std::vector<int> lookup_path) {
  std::string vname = absl::StrCat(message_name, ".", field->name());
  VName v_name = builder_->VNameForDescriptor(field);
  AddComments(v_name, lookup_path);

  {
    // Get location of declaration and add as Grok binding
    ScopedLookup name_num(&lookup_path, FieldDescriptorProto::kNameFieldNumber);
    const std::vector<int>& span = location_map_[lookup_path];
    Location location;
    InitializeLocation(span, &location);

    VName oneof;
    bool in_oneof = false;
    if (field->containing_oneof() != nullptr) {
      in_oneof = true;
      oneof = builder_->VNameForDescriptor(field->containing_oneof());
    }

    builder_->AddFieldToMessage(parent, message, in_oneof ? &oneof : nullptr,
                                v_name, location);
  }

  AttachMarkedSource(v_name, GenerateMarkedSourceForDescriptor(field));

  // Check for [deprecated=true] annotations and emit deprecation tags.
  if (field->options().deprecated()) {
    builder_->SetDeprecated(v_name);
  }

  Location type_location;
  {
    ScopedLookup type_num(&lookup_path,
                          FieldDescriptorProto::kTypeNameFieldNumber);
    if (location_map_.find(lookup_path) == location_map_.end()) {
      // the type was primitive, ignore for now
      return;
    }
    const std::vector<int>& type_span = location_map_[lookup_path];
    InitializeLocation(type_span, &type_location);

    // If we're in a message or enum type, decorate only the span
    // covering the type name itself, not the full package name.
    // This is consistent with other languages and avoids the possibility
    // of a multi-line span, which some UIs have problems with.
    TruncateLocationToTypeName(type_location, *field);
  }
  if (auto type = VNameForFieldType(field)) {
    // TODO: add value_type back in at some point.
    // Add reference for this field's type.  We assume it to be output
    // processing a dependency, but in the worst case this might introduce
    // an edge to no VName (presumably in turn introducing a Lost node).
    builder_->AddReference(*type, type_location);
    builder_->AddTyping(v_name, *type);
  }

  if (field->is_map()) {
    // Map key/value types do not have SourceCodeInfo locations; we have to
    // find them within the outer "map<...>" type location.
    absl::string_view content = absl::string_view(content_);
    absl::string_view type_name = content.substr(
        type_location.begin, type_location.end - type_location.begin);
    absl::string_view key, val;
    if (RE2::FullMatch(type_name, R"(\s*map\s*<\s*(\S+)\s*,\s*(\S+)\s*>\s*)",
                       &key, &val)) {
      // Add references to map type components.
      if (auto key_type = VNameForFieldType(field->message_type()->field(0))) {
        size_t key_start = key.data() - content.data();
        builder_->AddReference(
            *key_type, {type_location.file, key_start, key_start + key.size()});
      }

      if (auto val_type = VNameForFieldType(field->message_type()->field(1))) {
        size_t val_start = val.data() - content.data();
        builder_->AddReference(
            *val_type, {type_location.file, val_start, val_start + val.size()});
      }
      // TODO(schroederc): emit map type node
    }
  }

  if (field->has_default_value()) {
    const EnumValueDescriptor* default_value = field->default_value_enum();
    VName value = builder_->VNameForDescriptor(default_value);
    // Find reference location
    ScopedLookup default_num(&lookup_path,
                             FieldDescriptorProto::kDefaultValueFieldNumber);

    const std::vector<int>& value_span = location_map_[lookup_path];
    Location value_location;
    InitializeLocation(value_span, &value_location);
    builder_->AddReference(value, value_location);
  }
}

void FileDescriptorWalker::VisitFields(const std::string& message_name,
                                       const Descriptor* dp,
                                       std::vector<int> lookup_path) {
  VName message = VNameForProtoPath(file_name_, lookup_path);
  if (visited_messages_.find(URI(message).ToString()) !=
      visited_messages_.end()) {
    return;
  }
  visited_messages_.insert(URI(message).ToString());
  {
    ScopedLookup field_num(&lookup_path, DescriptorProto::kFieldFieldNumber);
    for (int i = 0; i < dp->field_count(); i++) {
      ScopedLookup field_index(&lookup_path, i);

      VisitField(&message_name, &message, message_name, message, dp->field(i),
                 lookup_path);
    }
  }
  {
    ScopedLookup extension_num(&lookup_path,
                               DescriptorProto::kExtensionFieldNumber);
    for (int i = 0; i < dp->extension_count(); i++) {
      ScopedLookup extension_index(&lookup_path, i);
      VisitExtension(&message_name, &message, dp->extension(i), lookup_path);
    }
  }
}

void FileDescriptorWalker::VisitNestedEnumTypes(const std::string& message_name,
                                                const VName* message,
                                                const Descriptor* dp,
                                                std::vector<int> lookup_path) {
  ScopedLookup enum_num(&lookup_path, DescriptorProto::kEnumTypeFieldNumber);
  for (int i = 0; i < dp->enum_type_count(); i++) {
    const EnumDescriptor* nested_proto = dp->enum_type(i);

    // Get the path that corresponds to the name of the enum
    ScopedLookup enum_index(&lookup_path, i);

    std::string vname = absl::StrCat(message_name, ".", nested_proto->name());

    VName v_name = builder_->VNameForDescriptor(nested_proto);
    AddComments(v_name, lookup_path);

    {
      ScopedLookup name_num(&lookup_path,
                            EnumDescriptorProto::kNameFieldNumber);
      const std::vector<int>& span = location_map_[lookup_path];
      Location location;
      InitializeLocation(span, &location);

      builder_->AddEnumType(message, v_name, location);
      if (nested_proto->options().deprecated()) {
        builder_->SetDeprecated(v_name);
      }
      AttachMarkedSource(v_name,
                         GenerateMarkedSourceForDescriptor(nested_proto));
    }

    // Visit values
    VisitEnumValues(nested_proto, &v_name, lookup_path);
  }
}

void FileDescriptorWalker::VisitNestedTypes(const std::string& message_name,
                                            const VName* message,
                                            const Descriptor* dp,
                                            std::vector<int> lookup_path) {
  ScopedLookup nested_type_num(&lookup_path,
                               DescriptorProto::kNestedTypeFieldNumber);

  for (int i = 0; i < dp->nested_type_count(); i++) {
    ScopedLookup nested_index(&lookup_path, i);
    const Descriptor* nested_proto = dp->nested_type(i);

    // The proto compiler synthesizes types to represent map entries. For
    // example, a "map<string, string> my_map" field would cause a type
    // "MyMapEntry" to be generated. Because it doesn't actually exist in the
    // source .proto file, we ignore it.
    if (nested_proto->options().map_entry()) {
      continue;
    }

    std::string vname = absl::StrCat(message_name, ".", nested_proto->name());

    VName v_name = VNameForProtoPath(file_name_, lookup_path);
    AddComments(v_name, lookup_path);

    {
      // Also push kNameFieldNumber for location of declaration
      ScopedLookup name_num(&lookup_path, DescriptorProto::kNameFieldNumber);

      const std::vector<int>& span = location_map_[lookup_path];
      Location location;
      InitializeLocation(span, &location);

      builder_->AddMessageType(message, v_name, location);
      if (nested_proto->options().deprecated()) {
        builder_->SetDeprecated(v_name);
      }
      AttachMarkedSource(v_name,
                         GenerateMarkedSourceForDescriptor(nested_proto));
    }

    // Need to visit nested enum and message types first!
    VisitNestedTypes(vname, &v_name, nested_proto, lookup_path);
    VisitNestedEnumTypes(vname, &v_name, nested_proto, lookup_path);
    VisitOneofs(vname, v_name, nested_proto, lookup_path);
  }
}

void FileDescriptorWalker::VisitOneofs(const std::string& message_name,
                                       const VName& message,
                                       const Descriptor* dp,
                                       std::vector<int> lookup_path) {
  ScopedLookup nested_type_num(&lookup_path,
                               DescriptorProto::kOneofDeclFieldNumber);

  for (int i = 0; i < dp->oneof_decl_count(); i++) {
    ScopedLookup nested_index(&lookup_path, i);
    const OneofDescriptor* oneof = dp->oneof_decl(i);
    std::string vname = absl::StrCat(message_name, ".", oneof->name());

    VName v_name = builder_->VNameForDescriptor(oneof);
    AddComments(v_name, lookup_path);

    {
      // TODO: verify that this is correct for oneofs
      ScopedLookup name_num(&lookup_path, DescriptorProto::kNameFieldNumber);

      const std::vector<int>& span = location_map_[lookup_path];
      Location location;
      InitializeLocation(span, &location);

      builder_->AddOneofToMessage(message, v_name, location);
      AttachMarkedSource(v_name, GenerateMarkedSourceForDescriptor(oneof));
    }

    // No need to add fields; they're also fields of the message
  }
}

void FileDescriptorWalker::VisitMessagesAndEnums(const std::string* ns_name,
                                                 const VName* ns) {
  std::vector<int> lookup_path;
  for (int i = 0; i < file_descriptor_->message_type_count(); i++) {
    ScopedLookup message_num(&lookup_path,
                             FileDescriptorProto::kMessageTypeFieldNumber);

    const Descriptor* dp = file_descriptor_->message_type(i);

    ScopedLookup message_index(&lookup_path, i);

    std::string vname = dp->name();
    if (ns_name != nullptr) {
      vname = absl::StrCat(*ns_name, ".", vname);
    }

    VName v_name = VNameForProtoPath(file_name_, lookup_path);
    AddComments(v_name, lookup_path);

    {
      ScopedLookup name_num(&lookup_path, DescriptorProto::kNameFieldNumber);
      const std::vector<int>& span = location_map_[lookup_path];
      Location location;
      InitializeLocation(span, &location);

      builder_->AddMessageType(ns, v_name, location);
      AttachMarkedSource(v_name, GenerateMarkedSourceForDescriptor(dp));
      if (dp->options().deprecated()) {
        builder_->SetDeprecated(v_name);
      }
    }

    // Visit nested types first and fields later for easy type resolution
    VisitNestedTypes(vname, &v_name, dp, lookup_path);
    VisitNestedEnumTypes(vname, &v_name, dp, lookup_path);
    VisitOneofs(vname, v_name, dp, lookup_path);
  }

  // Add top-level ENUM bindings
  for (int i = 0; i < file_descriptor_->enum_type_count(); i++) {
    ScopedLookup enum_num(&lookup_path,
                          FileDescriptorProto::kEnumTypeFieldNumber);
    const EnumDescriptor* dp = file_descriptor_->enum_type(i);
    ScopedLookup enum_index(&lookup_path, i);

    std::string vname = dp->name();
    if (ns_name != nullptr) {
      vname = absl::StrCat(*ns_name, ".", vname);
    }
    VName v_name = builder_->VNameForDescriptor(dp);
    AddComments(v_name, lookup_path);

    {
      ScopedLookup name_num(&lookup_path,
                            EnumDescriptorProto::kNameFieldNumber);
      const std::vector<int>& span = location_map_[lookup_path];
      Location location;
      InitializeLocation(span, &location);

      builder_->AddEnumType(ns, v_name, location);
      AttachMarkedSource(v_name, GenerateMarkedSourceForDescriptor(dp));
    }

    // Visit enum values and add kythe bindings for them
    VisitEnumValues(dp, &v_name, lookup_path);
  }
}

void FileDescriptorWalker::VisitEnumValues(const EnumDescriptor* dp,
                                           const VName* enum_node,
                                           std::vector<int> lookup_path) {
  ScopedLookup value_num(&lookup_path, EnumDescriptorProto::kValueFieldNumber);

  for (int j = 0; j < dp->value_count(); j++) {
    const EnumValueDescriptor* val_dp = dp->value(j);

    ScopedLookup value_index(&lookup_path, j);
    VName v_name = builder_->VNameForDescriptor(val_dp);
    AddComments(v_name, lookup_path);

    ScopedLookup name_num(&lookup_path,
                          EnumValueDescriptorProto::kNameFieldNumber);
    Location value_location;
    InitializeLocation(location_map_[lookup_path], &value_location);

    builder_->AddValueToEnum(*enum_node, v_name, value_location);
    if (val_dp->options().deprecated()) {
      builder_->SetDeprecated(v_name);
    }
    AttachMarkedSource(v_name, GenerateMarkedSourceForDescriptor(val_dp));
  }
}

void FileDescriptorWalker::VisitAllFields(const std::string* ns_name,
                                          const VName* ns) {
  std::vector<int> lookup_path;
  {
    ScopedLookup message_num(&lookup_path,
                             FileDescriptorProto::kMessageTypeFieldNumber);

    // For each top-level message in the file, add the field bindings
    for (int i = 0; i < file_descriptor_->message_type_count(); i++) {
      const Descriptor* dp = file_descriptor_->message_type(i);
      std::string vname = dp->name();
      if (ns_name != nullptr) {
        vname = *ns_name + "." + vname;
      }

      ScopedLookup message_index(&lookup_path, i);

      // Visit fields within the message
      VisitFields(vname, dp, lookup_path);

      // Visit fields in nested mesages
      VisitNestedFields(vname, dp, lookup_path);
    }
  }

  {
    ScopedLookup extension_num(&lookup_path,
                               FileDescriptorProto::kExtensionFieldNumber);

    // For each top-level extension in the file, add the field bindings
    for (int i = 0; i < file_descriptor_->extension_count(); i++) {
      ScopedLookup extension_index(&lookup_path, i);
      VisitExtension(ns_name, ns, file_descriptor_->extension(i), lookup_path);
    }
  }
}

void FileDescriptorWalker::VisitExtension(const std::string* parent_name,
                                          const VName* parent,
                                          const FieldDescriptor* field,
                                          std::vector<int> lookup_path) {
  std::string message_name = field->containing_type()->full_name();
  VName message = builder_->VNameForDescriptor(field->containing_type());
  {
    // In a block like this:
    // extend A {
    //    optional string b = 1;
    //    optional string c = 2;
    // }
    //
    // Link the name of the extended message "A" to the original
    // definition.  Each of "b" and "c" will generate this reference
    // which can result in duplicate references if more than one
    // field is declared in a single extend block.
    ScopedLookup extendee_num(&lookup_path,
                              FieldDescriptorProto::kExtendeeFieldNumber);
    const std::vector<int>& extendee_span = location_map_[lookup_path];
    Location extendee_location;
    InitializeLocation(extendee_span, &extendee_location);
    builder_->AddReference(message, extendee_location);
  }

  VisitField(parent_name, parent, message_name, message, field, lookup_path);
}

void FileDescriptorWalker::VisitNestedFields(const std::string& name_prefix,
                                             const Descriptor* dp,
                                             std::vector<int> lookup_path) {
  ScopedLookup nested_num(&lookup_path,
                          DescriptorProto::kNestedTypeFieldNumber);

  for (int j = 0; j < dp->nested_type_count(); j++) {
    const Descriptor* nested_dp = dp->nested_type(j);
    const std::string nested_name_prefix =
        absl::StrCat(name_prefix, ".", nested_dp->name());

    // The proto compiler synthesizes types to represent map entries. For
    // example, a "map<string, string> my_map" field would cause a type
    // "MyMapEntry" to be generated. Because it doesn't actually exist in the
    // source .proto file, we ignore it.
    if (nested_dp->options().map_entry()) {
      continue;
    }

    ScopedLookup nested_index(&lookup_path, j);

    // Visit fields within the message
    VisitFields(nested_name_prefix, nested_dp, lookup_path);

    // Visit fields in nested mesages
    VisitNestedFields(nested_name_prefix, nested_dp, lookup_path);
  }
}

void FileDescriptorWalker::AddComments(const VName& v_name,
                                       const std::vector<int>& path) {
  auto protoc_iter = path_location_map_.find(path);
  if (protoc_iter == path_location_map_.end()) {
    return;
  }
  const auto& protoc_location = protoc_iter->second;
  absl::StatusOr<PartialLocation> readable_location =
      ParseLocation(location_map_[path]);
  if (!readable_location.ok()) {
    return;
  }
  Location entity_location;
  InitializeLocation(location_map_[path], &entity_location);
  PartialLocation coordinates = *readable_location;
  if (protoc_location.has_leading_comments()) {
    Location comment_location = LocationOfLeadingComments(
        entity_location, coordinates.start_line, coordinates.start_column,
        protoc_location.leading_comments());
    builder_->AddDocComment(v_name, comment_location);
  }
  if (protoc_location.has_trailing_comments()) {
    Location comment_location = LocationOfTrailingComments(
        entity_location, coordinates.start_line, coordinates.start_column,
        protoc_location.trailing_comments());
    builder_->AddDocComment(v_name, comment_location);
  }
}

void FileDescriptorWalker::VisitRpcServices(const std::string* ns_name,
                                            const VName* ns) {
  std::vector<int> lookup_path;
  ScopedLookup service_num(&lookup_path,
                           FileDescriptorProto::kServiceFieldNumber);
  for (int i = 0; i < file_descriptor_->service_count(); i++) {
    const ServiceDescriptor* dp = file_descriptor_->service(i);
    ScopedLookup service_index(&lookup_path, i);

    std::string service_vname = dp->name();
    if (ns_name != nullptr) {
      service_vname = absl::StrCat(*ns_name, ".", service_vname);
    }
    VName v_name = builder_->VNameForDescriptor(dp);
    AddComments(v_name, lookup_path);

    {
      ScopedLookup name_num(&lookup_path,
                            ServiceDescriptorProto::kNameFieldNumber);
      const std::vector<int>& span = location_map_[lookup_path];
      Location location;
      InitializeLocation(span, &location);

      builder_->AddService(ns, v_name, location);
      AttachMarkedSource(v_name, GenerateMarkedSourceForDescriptor(dp));
    }

    // Visit methods
    ScopedLookup method_num(&lookup_path,
                            ServiceDescriptorProto::kMethodFieldNumber);
    for (int j = 0; j < dp->method_count(); j++) {
      const MethodDescriptor* method_dp = dp->method(j);
      ScopedLookup method_index(&lookup_path, j);
      std::string method_vname =
          absl::StrCat(service_vname, ".", method_dp->name());
      VName method = builder_->VNameForDescriptor(method_dp);
      AddComments(method, lookup_path);

      {
        // Add method as a declaration
        ScopedLookup name_num(&lookup_path,
                              MethodDescriptorProto::kNameFieldNumber);
        Location method_location;
        InitializeLocation(location_map_[lookup_path], &method_location);
        AttachMarkedSource(method,
                           GenerateMarkedSourceForDescriptor(method_dp));
        builder_->AddMethodToService(v_name, method, method_location);
      }

      {
        // Add rpc method's input argument
        ScopedLookup input_num(&lookup_path,
                               MethodDescriptorProto::kInputTypeFieldNumber);
        Location input_location;
        InitializeLocation(location_map_[lookup_path], &input_location);
        const Descriptor* input = method_dp->input_type();
        // Only decorate the type name, not the full <package>.<type> span.
        TruncateLocationToTypeName(input_location, *input);

        VName input_sig = builder_->VNameForDescriptor(input);
        builder_->AddArgumentToMethod(method, input_sig, input_location);
      }

      {
        // Add rpc method's output argument
        ScopedLookup output_num(&lookup_path,
                                MethodDescriptorProto::kOutputTypeFieldNumber);
        Location output_location;
        InitializeLocation(location_map_[lookup_path], &output_location);
        const Descriptor* output = method_dp->output_type();
        // Only decorate the type name, not the full <package>.<type> span.
        TruncateLocationToTypeName(output_location, *output);

        VName output_sig = builder_->VNameForDescriptor(output);
        builder_->AddArgumentToMethod(method, output_sig, output_location);
      }
    }
  }
}

void FileDescriptorWalker::PopulateCodeGraph() {
  BuildLocationMap(*source_code_info_);
  VisitGeneratedProtoInfo();
  VisitImports();

  const VName* ns = nullptr;
  const std::string* ns_name = nullptr;
  VName v_name;
  const std::string& package = file_descriptor_->package();
  if (!package.empty()) {
    std::vector<int> lookup_path;
    ScopedLookup package_num(&lookup_path,
                             FileDescriptorProto::kPackageFieldNumber);
    const std::vector<int>& span = location_map_[lookup_path];
    Location location;
    InitializeLocation(span, &location);
    v_name.set_language(kLanguageName);
    v_name.set_corpus(file_name_.corpus());
    v_name.set_signature(package);
    builder_->AddNamespace(v_name, location);
    ns = &v_name;
    ns_name = &package;
  }

  VisitMessagesAndEnums(ns_name, ns);
  VisitAllFields(ns_name, ns);
  VisitRpcServices(ns_name, ns);
}

}  // namespace lang_proto
}  // namespace kythe
