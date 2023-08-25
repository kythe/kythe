/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

#include "analyzer.h"

#include <algorithm>
#include <memory>
#include <optional>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/log/check.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor_database.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/cxx/common/utf8_line_index.h"
#include "kythe/cxx/extractor/textproto/textproto_schema.h"
#include "kythe/cxx/indexer/proto/offset_util.h"
#include "kythe/cxx/indexer/proto/search_path.h"
#include "kythe/cxx/indexer/proto/source_tree.h"
#include "kythe/cxx/indexer/proto/vname_util.h"
#include "kythe/cxx/indexer/textproto/plugin.h"
#include "kythe/cxx/indexer/textproto/recordio_textparser.h"
#include "kythe/proto/analysis.pb.h"
#include "re2/re2.h"

namespace kythe {
namespace lang_textproto {

ABSL_CONST_INIT const absl::string_view kLanguageName = "textproto";

namespace {

using ::google::protobuf::Descriptor;
using ::google::protobuf::DescriptorPool;
using ::google::protobuf::FieldDescriptor;
using ::google::protobuf::Message;
using ::google::protobuf::Reflection;
using ::google::protobuf::TextFormat;

// Repeated fields have an actual index, non-repeated fields are always -1.
constexpr int kNonRepeatedFieldIndex = -1;

// Error "collector" that just writes messages to log output.
class LoggingMultiFileErrorCollector
    : public google::protobuf::compiler::MultiFileErrorCollector {
 public:
  void AddError(const std::string& filename, int line, int column,
                const std::string& message) override {
    LOG(ERROR) << filename << "@" << line << ":" << column << ": " << message;
  }

  void AddWarning(const std::string& filename, int line, int column,
                  const std::string& message) override {
    LOG(WARNING) << filename << "@" << line << ":" << column << ": " << message;
  }
};

// Finds the file in the compilation unit's inputs and returns its vname.
// Returns an empty vname if the file is not found.
proto::VName LookupVNameForFullPath(absl::string_view full_path,
                                    const proto::CompilationUnit& unit) {
  for (const auto& input : unit.required_input()) {
    if (input.info().path() == full_path) {
      return input.v_name();
    }
  }
  LOG(ERROR) << "Unable to find file path in compilation unit: '" << full_path
             << "'. This likely indicates a bug in the textproto indexer, "
                "which should only need to construct VNames for files in "
                "the compilation unit";
  return proto::VName{};
}

// The TreeInfo contains the ParseInfoTree from proto2 textformat parser
// and line offset of the textproto within a file.
struct TreeInfo {
  const TextFormat::ParseInfoTree* parse_tree = nullptr;
  int line_offset = 0;
};

// The TextprotoAnalyzer maintains state needed across indexing operations and
// provides some relevant helper methods.
class TextprotoAnalyzer : public PluginApi {
 public:
  // Note: The TextprotoAnalyzer does not take ownership of its pointer
  // arguments, so they must outlive it.
  explicit TextprotoAnalyzer(
      const proto::CompilationUnit* unit, absl::string_view textproto,
      const absl::flat_hash_map<std::string, std::string>*
          file_substitution_cache,
      KytheGraphRecorder* recorder, const DescriptorPool* pool)
      : unit_(unit),
        recorder_(recorder),
        textproto_content_(textproto),
        line_index_(textproto),
        file_substitution_cache_(file_substitution_cache),
        descriptor_pool_(pool) {}

  // disallow copy and assign
  TextprotoAnalyzer(const TextprotoAnalyzer&) = delete;
  void operator=(const TextprotoAnalyzer&) = delete;

  // Recursively analyzes the message and any submessages, emitting "ref" edges
  // for all fields.
  absl::Status AnalyzeMessage(const proto::VName& file_vname,
                              const Message& proto,
                              const Descriptor& descriptor,
                              const TreeInfo& tree_info);

  // Analyzes the message contained inside a google.protobuf.Any field. The
  // parse location of the field (if nonzero) is used to add an anchor for the
  // Any's type specifier (i.e. [some.url/mypackage.MyMessage]).
  absl::Status AnalyzeAny(const proto::VName& file_vname, const Message& proto,
                          const Descriptor& descriptor,
                          const TreeInfo& tree_info,
                          TextFormat::ParseLocation field_loc);

  absl::StatusOr<proto::VName> AnalyzeAnyTypeUrl(
      const proto::VName& file_vname, TextFormat::ParseLocation field_loc);

  absl::Status AnalyzeEnumValue(const proto::VName& file_vname,
                                const FieldDescriptor& field, int start_offset);

  absl::Status AnalyzeStringValue(const proto::VName& file_vname,
                                  const Message& proto,
                                  const FieldDescriptor& field,
                                  int start_offset);
  absl::Status AnalyzeIntegerValue(const proto::VName& file_vname,
                                   const Message& proto,
                                   const FieldDescriptor& field,
                                   int start_offset);
  absl::Status AnalyzeSchemaComments(const proto::VName& file_vname,
                                     const Descriptor& msg_descriptor);

  KytheGraphRecorder* recorder() override { return recorder_; }

  void EmitDiagnostic(const proto::VName& file_vname,
                      absl::string_view signature,
                      absl::string_view msg) override;

  proto::VName CreateAndAddAnchorNode(const proto::VName& file, int begin,
                                      int end) override;

  proto::VName CreateAndAddAnchorNode(const proto::VName& file_vname,
                                      absl::string_view sp) override;

  proto::VName VNameForRelPath(
      absl::string_view simplified_path) const override;

  void SetPlugins(std::vector<std::unique_ptr<Plugin>> p) {
    plugins_ = std::move(p);
  }

  // Convenience method for constructing proto descriptor vnames.
  template <typename SomeDescriptor>
  proto::VName VNameForDescriptor(const SomeDescriptor* descriptor) {
    return ::kythe::lang_proto::VNameForDescriptor(
        descriptor, [this](auto path) { return VNameForRelPath(path); });
  }

  const DescriptorPool* ProtoDescriptorPool() const override {
    return descriptor_pool_;
  }

 private:
  absl::Status AnalyzeField(const proto::VName& file_vname,
                            const Message& proto, const TreeInfo& parse_tree,
                            const FieldDescriptor& field, int field_index);

  std::vector<StringToken> ReadStringTokens(absl::string_view input);

  int ComputeByteOffset(int line_number, int column_number) const;

  std::vector<std::unique_ptr<Plugin>> plugins_;

  const proto::CompilationUnit* unit_;
  KytheGraphRecorder* recorder_;
  const absl::string_view textproto_content_;
  const UTF8LineIndex line_index_;

  // Proto search paths are used to resolve relative paths to full paths.
  const absl::flat_hash_map<std::string, std::string>* file_substitution_cache_;
  // DescriptorPool is used to lookup descriptors for messages inside
  // protobuf.Any types.
  const DescriptorPool* descriptor_pool_;
};

// Converts from a proto line/column (both 0 based, and where column counts
// bytes except that tabs move to the next multiple of 8) to a byte offset
// from the start of the current file.  Returns -1 on error.
int TextprotoAnalyzer::ComputeByteOffset(int line_number,
                                         int column_number) const {
  int byte_offset_of_start_of_line =
      line_index_.ComputeByteOffset(line_number, 0);
  absl::string_view line_text = line_index_.GetLine(line_number);
  int byte_offset_into_line =
      lang_proto::ByteOffsetOfTabularColumn(line_text, column_number);
  if (byte_offset_into_line < 0) {
    return byte_offset_into_line;
  }
  return byte_offset_of_start_of_line + byte_offset_into_line;
}

proto::VName TextprotoAnalyzer::VNameForRelPath(
    absl::string_view simplified_path) const {
  absl::string_view full_path;
  auto it = file_substitution_cache_->find(simplified_path);
  if (it != file_substitution_cache_->end()) {
    full_path = it->second;
  } else {
    full_path = simplified_path;
  }
  return LookupVNameForFullPath(full_path, *unit_);
}

absl::Status TextprotoAnalyzer::AnalyzeMessage(const proto::VName& file_vname,
                                               const Message& proto,
                                               const Descriptor& descriptor,
                                               const TreeInfo& tree_info) {
  const Reflection* reflection = proto.GetReflection();

  // Iterate across all fields in the message. For proto1 and 2, each field has
  // a bit that tracks whether or not each field was set. This could be used to
  // only look at fields we know are set (with reflection.ListFields()). Proto3
  // however does not have "has" bits, so this approach would not work, thus we
  // look at every field.
  for (int field_index = 0; field_index < descriptor.field_count();
       field_index++) {
    const FieldDescriptor& field = *descriptor.field(field_index);
    if (field.is_repeated()) {
      const int count = reflection->FieldSize(proto, &field);
      if (count == 0) {
        continue;
      }

      // Add a ref for each instance of the repeated field.
      for (int i = 0; i < count; i++) {
        auto s = AnalyzeField(file_vname, proto, tree_info, field, i);
        if (!s.ok()) return s;
      }
    } else {
      auto s = AnalyzeField(file_vname, proto, tree_info, field,
                            kNonRepeatedFieldIndex);
      if (!s.ok()) return s;
    }
  }

  // Determine what extensions are present in the parsed proto and analyze them.
  std::vector<const FieldDescriptor*> set_fields;
  reflection->ListFields(proto, &set_fields);
  for (const FieldDescriptor* field : set_fields) {
    // Non-extensions are already handled above.
    if (!field->is_extension()) {
      continue;
    }

    if (field->is_repeated()) {
      const size_t count = reflection->FieldSize(proto, field);
      for (size_t i = 0; i < count; i++) {
        auto s = AnalyzeField(file_vname, proto, tree_info, *field, i);
        if (!s.ok()) return s;
      }
    } else {
      auto s = AnalyzeField(file_vname, proto, tree_info, *field,
                            kNonRepeatedFieldIndex);
      if (!s.ok()) return s;
    }
  }

  return absl::OkStatus();
}

// Given a type url that looks like "type.googleapis.com/example.Message1",
// returns "example.Message1".
std::string ProtoMessageNameFromAnyTypeUrl(absl::string_view type_url) {
  // Return the substring from after the last '/' to the end or an empty string.
  // If there is no slash, returns the entire string.
  return std::string(
      type_url.substr(std::min(type_url.size(), type_url.rfind('/') + 1)));
}

// Example textproto:
//   any_field {
//     [some.url/mypackage.MyMessage] {
//     }
//   }
//
// Given the start location of "any_field" as field_loc, this function uses a
// regex to find the "mypackage.MyMessage" portion and add an anchor node.
// Ideally this information would be provided in the ParseInfoTree generated by
// the textproto parser, but since it's not, we do our own "parsing" with a
// regex.
absl::StatusOr<proto::VName> TextprotoAnalyzer::AnalyzeAnyTypeUrl(
    const proto::VName& file_vname, TextFormat::ParseLocation field_loc) {
  // Note that line is 1-indexed; a value of zero indicates an empty location.
  if (field_loc.line == 0) return absl::OkStatus();

  absl::string_view sp = textproto_content_;
  const int search_from = ComputeByteOffset(field_loc.line, field_loc.column);
  sp = sp.substr(search_from);

  // Consume rest of field name, colon (optional) and open brace.
  if (!re2::RE2::Consume(&sp, R"(^[a-zA-Z0-9_]+:?\s*\{\s*)")) {
    return absl::UnknownError("");
  }
  // consume any extra comments before "[type_url]".
  while (re2::RE2::Consume(&sp, R"(\s*#.*\n*)")) {
  }
  // Regex for Any type url enclosed by square brackets, capturing just the
  // message name.
  absl::string_view match;
  if (!re2::RE2::PartialMatch(sp, R"(^\s*\[\s*[^/]+/([^\s\]]+)\s*\])",
                              &match)) {
    return absl::UnknownError("Unable to find type_url span for Any");
  }

  // Add anchor.
  return CreateAndAddAnchorNode(file_vname, match);
}

// When the textproto parser finds an Any message in the input, it parses the
// contained message and serializes it into an Any message. The any has a
// 'type_url' field describing the message type and a 'value' field containing
// the serialized bytes of the message. To analyze, we create a new instance of
// the message based on the type_url and de-serialize the value bytes into it.
// This is then passed to AnalyzeMessage, which does the actual analysis and
// matches fields up with the ParseInfoTree.
absl::Status TextprotoAnalyzer::AnalyzeAny(
    const proto::VName& file_vname, const Message& proto,
    const Descriptor& descriptor, const TreeInfo& tree_info,
    TextFormat::ParseLocation field_loc) {
  CHECK(descriptor.full_name() == "google.protobuf.Any");

  // Textproto usage of Any messages comes in two forms. You can specify the Any
  // directly via the `type_url` and `value` fields or you can specify the
  // message as a literal. If AnalyzeAnyTypeUrl() is unable to find a literal
  // starting with a type url enclosed in brackets, it returns an error and we
  // assume it's a directly-specified Any and defer to AnalyzeMessage.
  auto s = AnalyzeAnyTypeUrl(file_vname, field_loc);
  if (!s.ok()) {
    return AnalyzeMessage(file_vname, proto, descriptor, tree_info);
  }
  const proto::VName type_url_anchor = *s;

  const FieldDescriptor* type_url_desc = descriptor.FindFieldByName("type_url");
  const FieldDescriptor* value_desc = descriptor.FindFieldByName("value");
  if (type_url_desc == nullptr || value_desc == nullptr) {
    return absl::UnknownError("Unable to get field descriptors for Any");
  }

  const Reflection* reflection = proto.GetReflection();

  std::string type_url = reflection->GetString(proto, type_url_desc);
  std::string msg_name = ProtoMessageNameFromAnyTypeUrl(type_url);
  const Descriptor* msg_desc =
      descriptor_pool_->FindMessageTypeByName(msg_name);
  if (msg_desc == nullptr) {
    // Log the error, but continue. Failure to include the descriptor for an Any
    // shouldn't stop the rest of the file from being indexed.
    LOG(ERROR) << "Unable to find descriptor for message named " << msg_name;
    return absl::OkStatus();
  }

  // Add ref from type_url to proto message.
  auto msg_vname = VNameForDescriptor(msg_desc);
  recorder_->AddEdge(VNameRef(type_url_anchor), EdgeKindID::kRef,
                     VNameRef(msg_vname));

  // Deserialize Any value into the appropriate message type.
  std::string value_bytes = reflection->GetString(proto, value_desc);
  if (value_bytes.size() == 0) {
    // Any value is empty, nothing to index
    return absl::OkStatus();
  }
  google::protobuf::io::ArrayInputStream array_stream(value_bytes.data(),
                                                      value_bytes.size());
  google::protobuf::DynamicMessageFactory msg_factory;
  std::unique_ptr<Message> value_proto(
      msg_factory.GetPrototype(msg_desc)->New());
  google::protobuf::io::CodedInputStream coded_stream(&array_stream);
  if (!value_proto->ParseFromCodedStream(&coded_stream)) {
    return absl::UnknownError(absl::StrFormat(
        "Unable to parse Any.value bytes into a %s message", msg_name));
  }

  // Analyze the message contained in the Any.
  return AnalyzeMessage(file_vname, *value_proto, *msg_desc, tree_info);
}

// Trims whitespace (including newlines) and comments from the start of the
// input.
void ConsumeTextprotoWhitespace(absl::string_view* sp) {
  re2::RE2::Consume(sp, R"((\s+|#[^\n]*)*)");
}

// Adds an anchor and ref edge for usage of enum values. For example, in
// `my_enum_field: VALUE1`, this adds an anchor for "VALUE1".
absl::Status TextprotoAnalyzer::AnalyzeEnumValue(const proto::VName& file_vname,
                                                 const FieldDescriptor& field,
                                                 int start_offset) {
  // Start after the last character of the field name.
  absl::string_view input = textproto_content_;
  input = input.substr(start_offset);

  // Consume whitespace and colon after field name.
  ConsumeTextprotoWhitespace(&input);
  if (!re2::RE2::Consume(&input, ":")) {
    return absl::UnknownError("Failed to find ':' when analyzing enum value");
  }
  ConsumeTextprotoWhitespace(&input);

  // Detect 'array format' for repeated fields and trim the leading '['.
  const bool array_format =
      field.is_repeated() && re2::RE2::Consume(&input, "\\[");
  if (array_format) ConsumeTextprotoWhitespace(&input);

  while (true) {
    // Match the enum value, which may be an identifier or an integer.
    absl::string_view match;
    if (!re2::RE2::PartialMatch(input, R"(^([_\w\d]+))", &match)) {
      return absl::UnknownError("Failed to find text span for enum value: " +
                                field.full_name());
    }
    const std::string value_str(match);
    input = input.substr(value_str.size());

    // Lookup EnumValueDescriptor based on the matched value.
    const google::protobuf::EnumDescriptor* enum_field = field.enum_type();
    const google::protobuf::EnumValueDescriptor* enum_val =
        enum_field->FindValueByName(value_str);
    // If name lookup failed, try it as a number.
    if (!enum_val) {
      int value_int;
      if (!absl::SimpleAtoi(value_str, &value_int)) {
        return absl::InvalidArgumentError(
            absl::StrFormat("Unable to parse enum value: '%s'", value_str));
      }
      enum_val = enum_field->FindValueByNumber(value_int);
    }
    if (!enum_val) {
      return absl::InvalidArgumentError(
          absl::StrFormat("Unable to find enum value for '%s'", value_str));
    }

    // Add ref from matched text to enum value descriptor.
    proto::VName anchor_vname = CreateAndAddAnchorNode(file_vname, match);
    auto enum_vname = VNameForDescriptor(enum_val);
    recorder_->AddEdge(VNameRef(anchor_vname), EdgeKindID::kRef,
                       VNameRef(enum_vname));

    if (!array_format) break;

    // Consume trailing comma and whitespace; exit if there's no comma.
    ConsumeTextprotoWhitespace(&input);
    if (!re2::RE2::Consume(&input, ",")) {
      break;
    }
    ConsumeTextprotoWhitespace(&input);
  }

  return absl::OkStatus();
}

std::vector<StringToken> TextprotoAnalyzer::ReadStringTokens(
    absl::string_view input) {
  // Create a tokenizer for the input.
  google::protobuf::io::ArrayInputStream array_stream(input.data(),
                                                      input.size());
  google::protobuf::io::Tokenizer tokenizer(&array_stream, nullptr);
  // '#' starts a comment.
  tokenizer.set_comment_style(
      google::protobuf::io::Tokenizer::SH_COMMENT_STYLE);
  tokenizer.set_require_space_after_number(false);
  tokenizer.set_allow_multiline_strings(true);

  if (!tokenizer.Next() || tokenizer.current().type !=
                               google::protobuf::io::Tokenizer::TYPE_STRING) {
    return {};  // We require at least one string token.
  }

  // NOTE: the proto tokenizer uses 0-indexed line numbers, while UTF8LineIndex
  // expects them 1-indexed. Both use zero-indexed column numbers.
  const size_t start_offset = input.data() - textproto_content_.data();
  const size_t start_line = line_index_.LineNumber(start_offset);
  CharacterPosition start_pos =
      line_index_.ComputePositionForByteOffset(start_offset);
  CHECK(start_pos.line_number != -1);
  absl::string_view start_line_content =
      line_index_.GetLine(start_pos.line_number);
  const int start_col = start_pos.column_number;

  // Account for proto's tab behavior and its effect on what 'column number'
  // means :(.
  int proto_start_col = 0;
  for (int i = 0; i < start_col; ++i) {
    if (start_line_content[i] == '\t') {
      // tabs advance to the nearest 8th column
      proto_start_col += 8 - (proto_start_col % 8);
    } else {
      proto_start_col += 1;
    }
  }

  // Read all TYPE_STRING tokens.
  std::vector<StringToken> tokens;
  do {
    auto t = tokenizer.current();

    // adjust token line/col according to where we started the tokenizer.
    int column = t.column + (t.line == 0 ? proto_start_col : 0);
    int line = t.line + start_line;

    StringToken st;
    tokenizer.ParseStringAppend(t.text, &st.parsed_value);
    size_t token_offset = ComputeByteOffset(line, column);
    // create the string_view, trimming the first and last character, which are
    // quotes.
    st.source_text = absl::string_view(
        textproto_content_.data() + token_offset + 1, t.text.size() - 2);
    tokens.push_back(st);
  } while (tokenizer.Next() &&
           tokenizer.current().type ==
               google::protobuf::io::Tokenizer::TYPE_STRING);

  return tokens;
}

absl::Status TextprotoAnalyzer::AnalyzeStringValue(
    const proto::VName& file_vname, const Message& proto,
    const FieldDescriptor& field, int start_offset) {
  // Start after the last character of the field name.
  absl::string_view input = textproto_content_;
  input = input.substr(start_offset);

  // Consume rest of field name, colon (optional).
  ConsumeTextprotoWhitespace(&input);
  if (!re2::RE2::Consume(&input, ":")) {
    return absl::UnknownError("Failed to find ':' when analyzing string value");
  }
  ConsumeTextprotoWhitespace(&input);

  const bool array_format =
      field.is_repeated() && re2::RE2::Consume(&input, "\\[");
  if (array_format) ConsumeTextprotoWhitespace(&input);

  while (!input.empty()) {
    char c = input[0];
    if (c != '"' && c != '\'') {
      return absl::UnknownError("Can't find string");
    }

    std::vector<StringToken> tokens = ReadStringTokens(input);
    if (tokens.empty()) {
      return absl::UnknownError("Unable to find a string value for field: " +
                                field.name());
    }
    for (auto& p : plugins_) {
      auto s = p->AnalyzeStringField(this, file_vname, field, tokens);
      if (!s.ok()) {
        LOG(ERROR) << "Plugin error: " << s;
      }
    }
    // Advance `input` past the last string token we just parsed.
    const char* search_from = tokens.back().source_text.end() + 1;
    input = absl::string_view(search_from,
                              textproto_content_.end() - search_from + 1);

    if (!array_format) break;

    // Consume trailing comma and whitespace; exit if there's no comma.
    ConsumeTextprotoWhitespace(&input);
    if (!re2::RE2::Consume(&input, ",")) {
      break;
    }
    ConsumeTextprotoWhitespace(&input);
  }

  return absl::OkStatus();
}

absl::Status TextprotoAnalyzer::AnalyzeIntegerValue(
    const proto::VName& file_vname, const Message& proto,
    const FieldDescriptor& field, int start_offset) {
  // Start after the last character of the field name.
  absl::string_view input = textproto_content_;
  input = input.substr(start_offset);

  // Consume whitespace and colon after field name.
  ConsumeTextprotoWhitespace(&input);
  if (!re2::RE2::Consume(&input, ":")) {
    return absl::UnknownError(
        "Failed to find ':' when analyzing integer value");
  }
  ConsumeTextprotoWhitespace(&input);

  // Detect 'array format' for repeated fields and trim the leading '['.
  const bool array_format = field.is_repeated() && RE2::Consume(&input, "\\[");
  if (array_format) ConsumeTextprotoWhitespace(&input);

  while (true) {
    // Match the integer value.
    absl::string_view match;
    if (!re2::RE2::PartialMatch(input, R"(^([\d]+))", &match)) {
      return absl::UnknownError("Failed to find text span for enum value: " +
                                field.full_name());
    }
    input = input.substr(match.size());
    for (auto& p : plugins_) {
      auto s = p->AnalyzeIntegerField(this, file_vname, field, match);
      if (!s.ok()) {
        LOG(ERROR) << "Plugin error: " << s;
      }
    }

    if (!array_format) break;

    // Consume trailing comma and whitespace; exit if there's no comma.
    ConsumeTextprotoWhitespace(&input);
    if (!re2::RE2::Consume(&input, ",")) {
      break;
    }
    ConsumeTextprotoWhitespace(&input);
  }

  return absl::OkStatus();
}

// Analyzes the field and returns the number of values indexed. Typically this
// is 1, but it could be 1+ when list syntax is used in the textproto.
absl::Status TextprotoAnalyzer::AnalyzeField(const proto::VName& file_vname,
                                             const Message& proto,
                                             const TreeInfo& tree_info,
                                             const FieldDescriptor& field,
                                             int field_index) {
  TextFormat::ParseLocation loc =
      tree_info.parse_tree->GetLocation(&field, field_index);
  // Location of field that does not exists in the txt format returns -1.
  // GetLocation() returns 0-indexed values, but UTF8LineIndex expects
  // 1-indexed line numbers.
  loc.line += tree_info.line_offset + 1;

  bool add_anchor_node = true;
  if (loc.line == tree_info.line_offset) {
    // When AnalyzeField() is called for repeated fields or extensions, we know
    // the field was actually present in the input textproto. In the case of
    // repeated fields, the presence of only one location entry but multiple
    // values indicates that the shorthand/inline repeated field syntax was
    // used. The inline syntax looks like:
    //
    //   repeated_field: ["value1", "value2"]
    //
    // Versus the standard syntax:
    //
    //   repeated_field: "value1"
    //   repeated_field: "value2"
    //
    // This case is handled specially because there is only one "repeated_field"
    // to add an anchor node for, but each value is still analyzed individually.
    if (field_index != kNonRepeatedFieldIndex && field_index > 0) {
      // Inline/short-hand repeated field syntax was used. There is no
      // "field_name:" for this entry to add an anchor node for.
      add_anchor_node = false;
    } else if (field.is_extension() || field_index != kNonRepeatedFieldIndex) {
      // If we can't find a location for a set extension or the first entry of
      // the repeated field, this is a bug.
      return absl::UnknownError(
          absl::StrCat("Failed to find location of field: ", field.full_name(),
                       ". This is a bug in the textproto indexer."));
    } else {
      // Normal proto field. Failure to find a location just means it's not set.
      return absl::OkStatus();
    }
  }

  if (add_anchor_node) {
    const size_t len =
        field.is_extension() ? field.full_name().size() : field.name().size();
    if (field.is_extension()) {
      loc.column++;  // Skip leading "[" for extensions.
    }
    const int begin = ComputeByteOffset(loc.line, loc.column);
    const int end = begin + len;
    proto::VName anchor_vname = CreateAndAddAnchorNode(file_vname, begin, end);

    // Add ref/writes to proto field.
    auto field_vname = VNameForDescriptor(&field);
    recorder_->AddEdge(VNameRef(anchor_vname), EdgeKindID::kRefWrites,
                       VNameRef(field_vname));

    // Add refs for enum values.
    if (field.type() == FieldDescriptor::TYPE_ENUM) {
      auto s = AnalyzeEnumValue(file_vname, field, end);
      if (!s.ok()) {
        // Log this error, but don't block further progress
        LOG(ERROR) << "Error analyzing enum value: " << s;
      }
    } else if (field.type() == FieldDescriptor::TYPE_STRING &&
               !plugins_.empty()) {
      auto s = AnalyzeStringValue(file_vname, proto, field, end);
      if (!s.ok()) {
        LOG(ERROR) << "Error analyzing string value: " << s;
      }
    } else if (!plugins_.empty() &&
               (field.type() == FieldDescriptor::TYPE_FIXED32 ||
                field.type() == FieldDescriptor::TYPE_FIXED64 ||
                field.type() == FieldDescriptor::TYPE_UINT32 ||
                field.type() == FieldDescriptor::TYPE_UINT64 ||
                field.type() == FieldDescriptor::TYPE_INT32 ||
                field.type() == FieldDescriptor::TYPE_INT64)) {
      auto s = AnalyzeIntegerValue(file_vname, proto, field, end);
      if (!s.ok()) {
        LOG(ERROR) << "Error analyzing integer value: " << s;
      }
    }
  }

  // Handle submessage.
  if (field.type() == FieldDescriptor::TYPE_MESSAGE) {
    const TextFormat::ParseInfoTree* subtree =
        tree_info.parse_tree->GetTreeForNested(&field, field_index);
    if (subtree == nullptr) {
      return absl::OkStatus();
    }
    TreeInfo subtree_info{subtree, tree_info.line_offset};

    const Reflection* reflection = proto.GetReflection();
    const Message& submessage =
        field_index == kNonRepeatedFieldIndex
            ? reflection->GetMessage(proto, &field)
            : reflection->GetRepeatedMessage(proto, &field, field_index);
    const Descriptor& subdescriptor = *field.message_type();

    if (subdescriptor.full_name() == "google.protobuf.Any") {
      // The location of the field is used to find the location of the Any type
      // url and add an anchor node.
      TextFormat::ParseLocation field_loc =
          add_anchor_node ? loc : TextFormat::ParseLocation{};
      return AnalyzeAny(file_vname, submessage, subdescriptor, subtree_info,
                        field_loc);
    } else {
      return AnalyzeMessage(file_vname, submessage, subdescriptor,
                            subtree_info);
    }
  }

  return absl::OkStatus();
}

absl::Status TextprotoAnalyzer::AnalyzeSchemaComments(
    const proto::VName& file_vname, const Descriptor& msg_descriptor) {
  TextprotoSchema schema = ParseTextprotoSchemaComments(textproto_content_);

  // Handle 'proto-message' comment if present.
  if (!schema.proto_message.empty()) {
    size_t begin = schema.proto_message.begin() - textproto_content_.begin();
    size_t end = begin + schema.proto_message.size();
    proto::VName anchor = CreateAndAddAnchorNode(file_vname, begin, end);

    // Add ref edge to proto message.
    auto msg_vname = VNameForDescriptor(&msg_descriptor);
    recorder_->AddEdge(VNameRef(anchor), EdgeKindID::kRef, VNameRef(msg_vname));
  }

  // Handle 'proto-file' and 'proto-import' comments if present.
  std::vector<absl::string_view> proto_files = schema.proto_imports;
  if (!schema.proto_file.empty()) {
    proto_files.push_back(schema.proto_file);
  }
  for (const absl::string_view file : proto_files) {
    size_t begin = file.begin() - textproto_content_.begin();
    size_t end = begin + file.size();
    proto::VName anchor = CreateAndAddAnchorNode(file_vname, begin, end);

    // Add ref edge to file.
    proto::VName v = VNameForRelPath(file);
    recorder_->AddEdge(VNameRef(anchor), EdgeKindID::kRefFile, VNameRef(v));
  }

  return absl::OkStatus();
}

proto::VName TextprotoAnalyzer::CreateAndAddAnchorNode(
    const proto::VName& file_vname, int begin, int end) {
  proto::VName anchor = file_vname;
  anchor.set_language(std::string(kLanguageName));
  anchor.set_signature(absl::StrCat("@", begin, ":", end));

  recorder_->AddProperty(VNameRef(anchor), NodeKindID::kAnchor);
  recorder_->AddProperty(VNameRef(anchor), PropertyID::kLocationStartOffset,
                         begin);
  recorder_->AddProperty(VNameRef(anchor), PropertyID::kLocationEndOffset, end);

  return anchor;
}

// Adds an anchor node, using the string_view's offset relative to
// `textproto_content_` as the start location.
proto::VName TextprotoAnalyzer::CreateAndAddAnchorNode(
    const proto::VName& file_vname, absl::string_view sp) {
  CHECK(sp.begin() >= textproto_content_.begin() &&
        sp.end() <= textproto_content_.end())
      << "string_view not in range of source text";
  const int begin = sp.begin() - textproto_content_.begin();
  const int end = begin + sp.size();
  return CreateAndAddAnchorNode(file_vname, begin, end);
}

void TextprotoAnalyzer::EmitDiagnostic(const proto::VName& file_vname,
                                       absl::string_view signature,
                                       absl::string_view msg) {
  proto::VName dn_vname = file_vname;
  dn_vname.set_signature(std::string(signature));
  recorder_->AddProperty(VNameRef(dn_vname), NodeKindID::kDiagnostic);
  recorder_->AddProperty(VNameRef(dn_vname), PropertyID::kDiagnosticMessage,
                         msg);

  recorder_->AddEdge(VNameRef(file_vname), EdgeKindID::kTagged,
                     VNameRef(dn_vname));
}

// Find and return the argument after given argname. Removes the flag and
// argument from @args if found.
std::optional<std::string> FindArg(std::vector<std::string>* args,
                                   std::string argname) {
  for (auto iter = args->begin(); iter != args->end(); iter++) {
    if (*iter == argname) {
      if (iter + 1 < args->end()) {
        std::string v = *(iter + 1);
        args->erase(iter, iter + 2);
        return v;
      }
      return std::nullopt;
    }
  }
  return std::nullopt;
}

/// Given a full file path, returns a path relative to a directory in the
/// current search path. If the mapping isn't already in the cache, it is added.
/// \param full_path Full path to proto file
/// \param path_substitutions A map of (virtual directory, real directory) pairs
/// \param file_substitution_cache A map of (fullpath, relpath) pairs
std::string FullPathToRelative(
    const absl::string_view full_path,
    const std::vector<std::pair<std::string, std::string>>& path_substitutions,
    absl::flat_hash_map<std::string, std::string>* file_substitution_cache) {
  // If the SourceTree has opened this path already, its entry will be in the
  // cache.
  for (const auto& sub : *file_substitution_cache) {
    if (sub.second == full_path) {
      return sub.first;
    }
  }

  // Look through substitutions for a directory mapping that contains the given
  // full_path.
  // TODO(justbuchanan): consider using the *longest* match, not just the
  // first one.
  for (auto& sub : path_substitutions) {
    std::string dir = sub.second;
    if (!absl::EndsWith(dir, "/")) {
      dir += "/";
    }

    // If this substitution matches, apply it and return the simplified path.
    absl::string_view relpath = full_path;
    if (absl::ConsumePrefix(&relpath, dir)) {
      std::string result = sub.first.empty() ? std::string(relpath)
                                             : JoinPath(sub.first, relpath);
      (*file_substitution_cache)[result] = std::string(full_path);
      return result;
    }
  }

  return std::string(full_path);
}

}  // namespace

absl::Status AnalyzeCompilationUnit(const proto::CompilationUnit& unit,
                                    const std::vector<proto::FileData>& files,
                                    KytheGraphRecorder* recorder) {
  auto nil_loader = [](const google::protobuf::Message& proto)
      -> std::vector<std::unique_ptr<Plugin>> { return {}; };
  return AnalyzeCompilationUnit(nil_loader, unit, files, recorder);
}

absl::Status AnalyzeCompilationUnit(PluginLoadCallback plugin_loader,
                                    const proto::CompilationUnit& unit,
                                    const std::vector<proto::FileData>& files,
                                    KytheGraphRecorder* recorder) {
  if (unit.source_file().empty()) {
    return absl::FailedPreconditionError(
        "Expected Unit to contain 1+ source files");
  }
  if (files.size() < 2) {
    return absl::FailedPreconditionError(
        "Must provide at least 2 files: a textproto and 1+ .proto files");
  }

  absl::flat_hash_set<std::string> textproto_filenames;
  for (const std::string& filename : unit.source_file()) {
    textproto_filenames.insert(filename);
  }

  // Parse path substitutions from arguments.
  absl::flat_hash_map<std::string, std::string> file_substitution_cache;
  std::vector<std::pair<std::string, std::string>> path_substitutions;
  std::vector<std::string> args;
  ::kythe::lang_proto::ParsePathSubstitutions(unit.argument(),
                                              &path_substitutions, &args);

  // Find --proto_message in args.
  std::string message_name = FindArg(&args, "--proto_message").value_or("");
  if (message_name.empty()) {
    return absl::UnknownError(
        "Compilation unit arguments must specify --proto_message");
  }
  LOG(INFO) << "Proto message name: " << message_name;

  absl::flat_hash_map<std::string, const proto::FileData*> file_data_by_path;

  // Load all proto files into in-memory SourceTree.
  PreloadedProtoFileTree file_reader(&path_substitutions,
                                     &file_substitution_cache);
  std::vector<std::string> proto_filenames;
  for (const auto& file : files) {
    // Skip textproto - only proto files go in the descriptor db.
    if (textproto_filenames.find(file.info().path()) !=
        textproto_filenames.end()) {
      file_data_by_path[file.info().path()] = &file;
      continue;
    }

    DLOG(LEVEL(-1)) << "Added file to descriptor db: " << file.info().path();
    if (!file_reader.AddFile(file.info().path(), file.content())) {
      return absl::UnknownError("Unable to add file to SourceTree.");
    }
    proto_filenames.push_back(file.info().path());
  }
  if (textproto_filenames.size() != file_data_by_path.size()) {
    return absl::NotFoundError(
        "Couldn't find all textproto sources in file data.");
  }

  // Build proto descriptor pool with top-level protos.
  LoggingMultiFileErrorCollector error_collector;
  google::protobuf::compiler::Importer proto_importer(&file_reader,
                                                      &error_collector);
  for (const std::string& fname : proto_filenames) {
    // The proto importer gets confused if the same proto file is Import()'d
    // under two different file paths. For example, if subdir/some.proto is
    // imported as "subdir/some.proto" in one place and "some.proto" in another
    // place, the importer will see duplicate symbol definitions and fail. To
    // work around this, we use relative paths for importing because the
    // "import" statements in proto files are also relative to the proto
    // compiler search path. This ensures that the importer doesn't see the same
    // file twice under two different names.
    std::string relpath =
        FullPathToRelative(fname, path_substitutions, &file_substitution_cache);
    if (!proto_importer.Import(relpath)) {
      return absl::UnknownError("Error importing proto file: " + relpath);
    }
    DLOG(LEVEL(-1)) << "Added proto to descriptor pool: " << relpath;
  }
  const DescriptorPool* descriptor_pool = proto_importer.pool();

  // Get a descriptor for the top-level Message.
  const Descriptor* descriptor =
      descriptor_pool->FindMessageTypeByName(message_name);
  if (descriptor == nullptr) {
    return absl::NotFoundError(absl::StrCat(
        "Unable to find proto message in descriptor pool: ", message_name));
  }

  // Only recordio format specifies record_separator.
  // Presense of record_separator flag indicates it's recordio file format.
  std::optional<std::string> record_separator =
      FindArg(&args, "--record_separator");
  for (auto& [filepath, filecontent] : file_data_by_path) {
    // Use reflection to create an instance of the top-level proto message.
    // note: msg_factory must outlive any protos created from it.
    google::protobuf::DynamicMessageFactory msg_factory;
    std::unique_ptr<Message> proto(msg_factory.GetPrototype(descriptor)->New());

    // Emit file node.
    proto::VName file_vname = LookupVNameForFullPath(filepath, unit);
    recorder->AddProperty(VNameRef(file_vname), NodeKindID::kFile);
    // Record source text as a fact.
    recorder->AddProperty(VNameRef(file_vname), PropertyID::kText,
                          filecontent->content());

    TextprotoAnalyzer analyzer(&unit, filecontent->content(),
                               &file_substitution_cache, recorder,
                               descriptor_pool);

    // Load plugins
    analyzer.SetPlugins(plugin_loader(*proto));

    absl::Status status =
        analyzer.AnalyzeSchemaComments(file_vname, *descriptor);
    if (!status.ok()) {
      std::string msg =
          absl::StrCat("Error analyzing schema comments: ", status.ToString());
      LOG(ERROR) << msg << status;
      analyzer.EmitDiagnostic(file_vname, "schema_comments", msg);
    }

    TextFormat::Parser parser;
    // Relax parser restrictions - even if the proto is partially ill-defined,
    // we'd like to analyze the parts that are good.
    parser.AllowPartialMessage(true);
    parser.AllowUnknownExtension(true);

    auto analyze_message = [&](absl::string_view chunk, int start_line) {
      LOG(INFO) << "Analyze chunk at line: " << start_line;
      // Parse textproto into @proto, recording input locations to @parse_tree.
      TextFormat::ParseInfoTree parse_tree;
      parser.WriteLocationsTo(&parse_tree);

      google::protobuf::io::ArrayInputStream stream(chunk.data(), chunk.size());
      if (!parser.Parse(&stream, proto.get())) {
        return absl::UnknownError("Failed to parse text proto");
      }

      TreeInfo tree_info{&parse_tree, start_line};
      return analyzer.AnalyzeMessage(file_vname, *proto, *descriptor,
                                     tree_info);
    };

    if (record_separator.has_value()) {
      LOG(INFO) << "Analyzing recordio fileformat with delimiter: "
                << *record_separator;
      kythe::lang_textproto::ParseRecordTextChunks(
          filecontent->content(), *record_separator,
          [&](absl::string_view chunk, int line_offset) {
            absl::Status status = analyze_message(chunk, line_offset);
            if (!status.ok()) {
              LOG(ERROR) << "Failed to parse record starting at line "
                         << line_offset << ": " << status;
            }
          });
    } else {
      absl::Status status = analyze_message(filecontent->content(), 0);
      if (!status.ok()) {
        return status;
      }
    }
  }

  return absl::OkStatus();
}

}  // namespace lang_textproto
}  // namespace kythe
