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

#include <memory>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/match.h"
#include "absl/strings/strip.h"
#include "absl/types/optional.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor_database.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/text_format.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/cxx/common/status_or.h"
#include "kythe/cxx/common/utf8_line_index.h"
#include "kythe/cxx/extractor/textproto/textproto_schema.h"
#include "kythe/cxx/indexer/proto/search_path.h"
#include "kythe/cxx/indexer/proto/source_tree.h"
#include "kythe/cxx/indexer/proto/vname_util.h"
#include "kythe/proto/analysis.pb.h"

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

absl::optional<proto::VName> LookupVNameForFullPath(
    absl::string_view full_path, const proto::CompilationUnit& unit) {
  for (const auto& input : unit.required_input()) {
    if (input.info().path() == full_path) {
      return input.v_name();
    }
  }
  return absl::nullopt;
}

// The TextprotoAnalyzer maintains state needed across indexing operations and
// provides some relevant helper methods.
class TextprotoAnalyzer {
 public:
  // Note: The TextprotoAnalyzer does not take ownership of its pointer
  // arguments, so they must outlive it.
  explicit TextprotoAnalyzer(
      const proto::CompilationUnit* unit, absl::string_view textproto,
      const absl::flat_hash_map<std::string, std::string>*
          file_substitution_cache,
      KytheGraphRecorder* recorder)
      : unit_(unit),
        recorder_(recorder),
        textproto_content_(textproto),
        line_index_(textproto),
        file_substitution_cache_(file_substitution_cache) {}

  // disallow copy and assign
  TextprotoAnalyzer(const TextprotoAnalyzer&) = delete;
  void operator=(const TextprotoAnalyzer&) = delete;

  // Recursively analyzes the message and any submessages, emitting "ref" edges
  // for all fields.
  Status AnalyzeMessage(const proto::VName& file_vname, const Message& proto,
                        const Descriptor& descriptor,
                        const TextFormat::ParseInfoTree& parse_tree);

  Status AnalyzeSchemaComments(const proto::VName& file_vname,
                               const Descriptor& msg_descriptor);

  void EmitDiagnostic(const proto::VName& file_vname,
                      absl::string_view signature, absl::string_view msg);

 private:
  Status AnalyzeField(const proto::VName& file_vname, const Message& proto,
                      const TextFormat::ParseInfoTree& parse_tree,
                      const FieldDescriptor& field, int field_index);

  proto::VName CreateAndAddAnchorNode(const proto::VName& file, int begin,
                                      int end);

  absl::optional<proto::VName> VNameForRelPath(
      absl::string_view simplified_path) const;

  template <typename SomeDescriptor>
  StatusOr<proto::VName> VNameForDescriptor(const SomeDescriptor* descriptor) {
    Status vname_lookup_status = OkStatus();
    proto::VName vname = ::kythe::lang_proto::VNameForDescriptor(
        descriptor, [this, &vname_lookup_status](const std::string& path) {
          auto v = VNameForRelPath(path);
          if (!v.has_value()) {
            vname_lookup_status = UnknownError(
                absl::StrCat("Unable to lookup vname for rel path: ", path));
            return proto::VName();
          }
          return *v;
        });
    return vname_lookup_status.ok() ? StatusOr<proto::VName>(vname)
                                    : vname_lookup_status;
  }

  const proto::CompilationUnit* unit_;
  KytheGraphRecorder* recorder_;
  const absl::string_view textproto_content_;
  const UTF8LineIndex line_index_;

  // Proto search paths are used to resolve relative paths to full paths.
  const absl::flat_hash_map<std::string, std::string>* file_substitution_cache_;
};

absl::optional<proto::VName> TextprotoAnalyzer::VNameForRelPath(
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

Status TextprotoAnalyzer::AnalyzeMessage(
    const proto::VName& file_vname, const Message& proto,
    const Descriptor& descriptor, const TextFormat::ParseInfoTree& parse_tree) {
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
      // Handle repeated field.
      const int count = reflection->FieldSize(proto, &field);
      if (count == 0) {
        continue;
      }

      // Add a ref for each instance of the repeated field.
      for (int i = 0; i < count; i++) {
        auto s = AnalyzeField(file_vname, proto, parse_tree, field, i);
        if (!s.ok()) return s;
      }
    } else {
      auto s = AnalyzeField(file_vname, proto, parse_tree, field,
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
        auto s = AnalyzeField(file_vname, proto, parse_tree, *field, i);
        if (!s.ok()) return s;
      }
    } else {
      auto s = AnalyzeField(file_vname, proto, parse_tree, *field,
                            kNonRepeatedFieldIndex);
      if (!s.ok()) return s;
    }
  }

  return OkStatus();
}

Status TextprotoAnalyzer::AnalyzeField(
    const proto::VName& file_vname, const Message& proto,
    const TextFormat::ParseInfoTree& parse_tree, const FieldDescriptor& field,
    int field_index) {
  TextFormat::ParseLocation loc = parse_tree.GetLocation(&field, field_index);
  // GetLocation() returns 0-indexed values, but UTF8LineIndex expects
  // 1-indexed line numbers.
  loc.line++;

  bool add_anchor_node = true;
  if (loc.line == 0) {
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
      return UnknownError(
          absl::StrCat("Failed to find location of field: ", field.full_name(),
                       ". This is a bug in the textproto indexer."));
    } else {
      // Normal proto field. Failure to find a location just means it's not set.
      return OkStatus();
    }
  }

  if (add_anchor_node) {
    const size_t len =
        field.is_extension() ? field.full_name().size() : field.name().size();
    if (field.is_extension()) {
      loc.column++;  // Skip leading "[" for extensions.
    }
    const int begin = line_index_.ComputeByteOffset(loc.line, loc.column);
    const int end = begin + len;
    proto::VName anchor_vname = CreateAndAddAnchorNode(file_vname, begin, end);

    // Add ref to proto field.
    auto field_vname = VNameForDescriptor(&field);
    if (!field_vname.ok()) return field_vname.status();
    recorder_->AddEdge(VNameRef(anchor_vname), EdgeKindID::kRef,
                       VNameRef(*field_vname));
  }

  // Handle submessage.
  if (field.type() == FieldDescriptor::TYPE_MESSAGE) {
    const TextFormat::ParseInfoTree& subtree =
        *parse_tree.GetTreeForNested(&field, field_index);
    const Reflection* reflection = proto.GetReflection();
    const Message& submessage =
        field_index == kNonRepeatedFieldIndex
            ? reflection->GetMessage(proto, &field)
            : reflection->GetRepeatedMessage(proto, &field, field_index);
    const Descriptor& subdescriptor = *field.message_type();
    auto s = AnalyzeMessage(file_vname, submessage, subdescriptor, subtree);
    if (!s.ok()) return s;
  }

  return OkStatus();
}

Status TextprotoAnalyzer::AnalyzeSchemaComments(
    const proto::VName& file_vname, const Descriptor& msg_descriptor) {
  TextprotoSchema schema = ParseTextprotoSchemaComments(textproto_content_);

  // Handle 'proto-message' comment if present.
  if (!schema.proto_message.empty()) {
    size_t begin = schema.proto_message.begin() - textproto_content_.begin();
    size_t end = begin + schema.proto_message.size();
    proto::VName anchor = CreateAndAddAnchorNode(file_vname, begin, end);

    // Add ref edge to proto message.
    auto msg_vname = VNameForDescriptor(&msg_descriptor);
    if (!msg_vname.ok()) return msg_vname.status();
    recorder_->AddEdge(VNameRef(anchor), EdgeKindID::kRef,
                       VNameRef(*msg_vname));
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
    auto v = VNameForRelPath(file);
    if (!v.has_value()) {
      return UnknownError(
          absl::StrCat("Unable to lookup vname for rel path: ", file));
    }
    recorder_->AddEdge(VNameRef(anchor), EdgeKindID::kRef, VNameRef(*v));
  }

  return OkStatus();
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

// Find and return the argument after --proto_message. Removes the flag and
// argument from @args if found.
absl::optional<std::string> ParseProtoMessageArg(
    std::vector<std::string>* args) {
  for (size_t i = 0; i < args->size(); i++) {
    if (args->at(i) == "--proto_message") {
      if (i + 1 < args->size()) {
        std::string v = args->at(i + 1);
        args->erase(args->begin() + i, args->begin() + i + 2);
        return v;
      }
      return absl::nullopt;
    }
  }
  return absl::nullopt;
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

Status AnalyzeCompilationUnit(const proto::CompilationUnit& unit,
                              const std::vector<proto::FileData>& files,
                              KytheGraphRecorder* recorder) {
  if (unit.source_file().size() != 1) {
    return FailedPreconditionError("Expected Unit to contain 1 source file");
  }
  if (files.size() < 2) {
    return FailedPreconditionError(
        "Must provide at least 2 files: a textproto and 1+ .proto files");
  }

  const std::string textproto_name = unit.source_file(0);

  // Parse path substitutions from arguments.
  absl::flat_hash_map<std::string, std::string> file_substitution_cache;
  std::vector<std::pair<std::string, std::string>> path_substitutions;
  std::vector<std::string> args;
  ::kythe::lang_proto::ParsePathSubstitutions(unit.argument(),
                                              &path_substitutions, &args);

  // Find --proto_message in args.
  std::string message_name;
  {
    auto opt_message_name = ParseProtoMessageArg(&args);
    if (!opt_message_name.has_value()) {
      return UnknownError(
          "Compilation unit arguments must specify --proto_message");
    }
    message_name = *opt_message_name;
  }
  LOG(INFO) << "Proto message name: " << message_name;

  // Load all proto files into in-memory SourceTree.
  PreloadedProtoFileTree file_reader(&path_substitutions,
                                     &file_substitution_cache);
  std::vector<std::string> proto_filenames;
  const proto::FileData* textproto_file_data = nullptr;
  for (const auto& file : files) {
    // Skip textproto - only proto files go in the descriptor db.
    if (file.info().path() == textproto_name) {
      textproto_file_data = &file;
      continue;
    }

    VLOG(1) << "Added file to descriptor db: " << file.info().path();
    if (!file_reader.AddFile(file.info().path(), file.content())) {
      return UnknownError("Unable to add file to SourceTree.");
    }
    proto_filenames.push_back(file.info().path());
  }
  if (textproto_file_data == nullptr) {
    return NotFoundError("Couldn't find textproto source in file data.");
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
      return UnknownError("Error importing proto file: " + relpath);
    }
    VLOG(1) << "Added proto to descriptor pool: " << relpath;
  }
  const DescriptorPool* descriptor_pool = proto_importer.pool();

  // Get a descriptor for the top-level Message.
  const Descriptor* descriptor =
      descriptor_pool->FindMessageTypeByName(message_name);
  if (descriptor == nullptr) {
    return NotFoundError(absl::StrCat(
        "Unable to find proto message in descriptor pool: ", message_name));
  }

  // Use reflection to create an instance of the top-level proto message.
  // note: msg_factory must outlive any protos created from it.
  google::protobuf::DynamicMessageFactory msg_factory;
  std::unique_ptr<Message> proto(msg_factory.GetPrototype(descriptor)->New());

  // Parse textproto into @proto, recording input locations to @parse_tree.
  TextFormat::ParseInfoTree parse_tree;
  {
    TextFormat::Parser parser;
    parser.WriteLocationsTo(&parse_tree);
    // Relax parser restrictions - even if the proto is partially ill-defined,
    // we'd like to analyze the parts that are good.
    parser.AllowPartialMessage(true);
    parser.AllowUnknownExtension(true);
    if (!parser.ParseFromString(textproto_file_data->content(), proto.get())) {
      return UnknownError("Failed to parse text proto");
    }
  }

  // Emit file node.
  absl::optional<proto::VName> file_vname =
      LookupVNameForFullPath(textproto_name, unit);
  if (!file_vname.has_value()) {
    return UnknownError(
        absl::StrCat("Unable to find vname for textproto: ", textproto_name));
  }
  recorder->AddProperty(VNameRef(*file_vname), NodeKindID::kFile);
  // Record source text as a fact.
  recorder->AddProperty(VNameRef(*file_vname), PropertyID::kText,
                        textproto_file_data->content());

  // Analyze!
  TextprotoAnalyzer analyzer(&unit, textproto_file_data->content(),
                             &file_substitution_cache, recorder);

  Status status = analyzer.AnalyzeSchemaComments(*file_vname, *descriptor);
  if (!status.ok()) {
    std::string msg =
        absl::StrCat("Error analyzing schema comments: ", status.ToString());
    LOG(ERROR) << msg << status;
    analyzer.EmitDiagnostic(*file_vname, "schema_comments", msg);
  }

  return analyzer.AnalyzeMessage(*file_vname, *proto, *descriptor, parse_tree);
}

}  // namespace lang_textproto
}  // namespace kythe
