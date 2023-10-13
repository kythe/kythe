/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/common/kythe_metadata_file.h"

#include <cctype>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/log/log.h"
#include "absl/strings/escaping.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "google/protobuf/util/json_util.h"
#include "kythe/cxx/common/json_proto.h"
#include "kythe/cxx/common/schema/edges.h"
#include "kythe/proto/metadata.pb.h"
#include "kythe/proto/storage.pb.h"

namespace kythe {
namespace {
bool CheckVName(const proto::VName& vname) {
  if (vname.corpus().empty() && vname.path().empty() && vname.root().empty() &&
      vname.signature().empty() && vname.language().empty()) {
    LOG(WARNING) << "When loading metadata: empty vname.";
    return false;
  }
  return true;
}

/// \brief Reads the contents of a C or C++ comment.
/// \param buf_string a buffer containing the text to read.
/// \param comment_slash_pos the offset of the first / starting the comment
/// in buf_string
/// \param data_start_pos the offset of the first byte of payload in
/// buf_string
std::optional<std::string> LoadCommentMetadata(absl::string_view buf_string,
                                               size_t comment_slash_pos,
                                               size_t data_start_pos) {
  std::string raw_data;
  // Over-reserves--though we expect the comment to be the only thing in the
  // file or the last thing in the file, so this approximation is reasonable.
  raw_data.reserve(buf_string.size() - comment_slash_pos);
  size_t pos = data_start_pos;
  // Tolerate single-line comments as well as multi-line comments.
  // If there's a single-line comment, it should be the only thing in the
  // file.
  bool single_line = buf_string[comment_slash_pos + 1] == '/';
  auto next_term =
      single_line ? absl::string_view::npos : buf_string.find("*/", pos);
  for (; pos < buf_string.size();) {
    while (pos < buf_string.size() && isspace(buf_string[pos])) ++pos;
    auto next_newline = buf_string.find('\n', pos);
    if (next_term != absl::string_view::npos &&
        (next_newline == absl::string_view::npos || next_term < next_newline)) {
      raw_data.append(buf_string.data() + pos, next_term - pos);
    } else if (next_newline != absl::string_view::npos) {
      raw_data.append(buf_string.data() + pos, next_newline - pos);
      pos = next_newline + 1;
      if (!single_line) {
        continue;
      }
    } else {
      raw_data.append(buf_string.data() + pos, buf_string.size() - pos);
    }
    break;
  }
  std::string decoded;
  return absl::Base64Unescape(raw_data, &decoded)
             ? std::optional<std::string>(std::string(decoded))
             : std::nullopt;
}

/// \brief Attempts to load buffer as a header-style metadata file.
/// \param buffer data to try and parse.
/// \return the decoded metadata on success or std::nullopt on failure.
std::optional<std::string> LoadHeaderMetadata(absl::string_view buffer) {
  if (buffer.size() < 2) {
    return std::nullopt;
  }
  auto buf_string = buffer.data();
  // If the header doesn't start with a comment, it's invalid.
  if (buf_string[0] != '/' || !(buf_string[1] == '*' || buf_string[1] == '/')) {
    return std::nullopt;
  }
  return LoadCommentMetadata(buf_string, 0, 2);
}

/// \brief Attempts to load buffer as an inline metadata file
/// \param buffer data to try and parse.
/// \param search_string the string identifying the data.
/// \return the decoded metadata on success or std::nullopt on failure.
std::optional<std::string> FindCommentMetadata(
    absl::string_view buffer, const std::string& search_string) {
  auto comment_start = buffer.find("/* " + search_string);
  if (comment_start == absl::string_view::npos) {
    comment_start = buffer.find("// " + search_string);
    if (comment_start == absl::string_view::npos) {
      return std::nullopt;
    }
  }
  // Data starts after the comment token, a space, and the user-provided
  // marker.
  return LoadCommentMetadata(buffer, comment_start,
                             comment_start + 3 + search_string.size());
}
}  // anonymous namespace

std::optional<MetadataFile::Rule> MetadataFile::LoadMetaElement(
    const proto::metadata::MappingRule& mapping) {
  using ::kythe::proto::metadata::MappingRule;
  if (mapping.type() == MappingRule::NOP) {
    return MetadataFile::Rule{};
  }

  absl::string_view edge_string = mapping.edge();
  if (edge_string.empty() && !(mapping.type() == MappingRule::ANCHOR_DEFINES &&
                               mapping.semantic() != MappingRule::SEMA_NONE)) {
    LOG(WARNING) << "When loading metadata: empty edge.";
    return std::nullopt;
  }
  bool reverse_edge = absl::ConsumePrefix(&edge_string, "%");
  if (mapping.type() == MappingRule::ANCHOR_DEFINES) {
    if (!CheckVName(mapping.vname())) {
      return std::nullopt;
    }
    Semantic sema;
    switch (mapping.semantic()) {
      case MappingRule::SEMA_WRITE:
        sema = Semantic::kWrite;
        break;
      case MappingRule::SEMA_READ_WRITE:
        sema = Semantic::kReadWrite;
        break;
      case MappingRule::SEMA_TAKE_ALIAS:
        sema = Semantic::kTakeAlias;
        break;
      default:
        sema = Semantic::kNone;
    }
    return MetadataFile::Rule{mapping.begin(),
                              mapping.end(),
                              kythe::common::schema::kDefinesBinding,
                              std::string(edge_string),
                              mapping.vname(),
                              reverse_edge,
                              false,
                              0,
                              0,
                              false,
                              sema};
  } else if (mapping.type() == MappingRule::ANCHOR_ANCHOR) {
    if (!CheckVName(mapping.source_vname())) {
      return std::nullopt;
    }
    return MetadataFile::Rule{mapping.target_begin(),
                              mapping.target_end(),
                              kythe::common::schema::kDefinesBinding,
                              std::string(edge_string),
                              mapping.source_vname(),
                              !reverse_edge,
                              true,
                              mapping.source_begin(),
                              mapping.source_end()};
  } else {
    LOG(WARNING) << "When loading metadata: unknown meta type "
                 << mapping.type();
    return std::nullopt;
  }
}

std::unique_ptr<MetadataFile> KytheMetadataSupport::LoadFromJSON(
    absl::string_view id, absl::string_view json) {
  proto::metadata::GeneratedCodeInfo metadata;
  google::protobuf::util::JsonParseOptions options;
  // Existing implementations specify message types using lower-case enum names,
  // so ensure we can parse those.
  options.case_insensitive_enum_parsing = true;
  auto status = ParseFromJsonString(json, options, &metadata);
  if (!status.ok()) {
    LOG(WARNING) << "Error parsing JSON metadata: " << status;
    return nullptr;
  }

  std::vector<MetadataFile::Rule> rules;
  rules.reserve(metadata.meta().size());
  for (const auto& meta_element : metadata.meta()) {
    if (auto rule = MetadataFile::LoadMetaElement(meta_element)) {
      rules.push_back(*std::move(rule));
    } else {
      return nullptr;
    }
  }
  return MetadataFile::LoadFromRules(id, rules.begin(), rules.end());
}

std::unique_ptr<kythe::MetadataFile> KytheMetadataSupport::ParseFile(
    const std::string& raw_filename, const std::string& filename,
    absl::string_view buffer, absl::string_view target_buffer) {
  auto metadata = LoadFromJSON(raw_filename, buffer);
  if (!metadata) {
    LOG(WARNING) << "Failed loading " << raw_filename;
  }
  return metadata;
}

void MetadataSupports::UseVNameLookup(VNameLookup lookup) const {
  for (auto& support : supports_) {
    support->UseVNameLookup(lookup);
  }
}

std::unique_ptr<kythe::MetadataFile> MetadataSupports::ParseFile(
    const std::string& filename, absl::string_view buffer,
    const std::string& search_string, absl::string_view target_buffer) const {
  std::string modified_filename = filename;
  std::optional<std::string> decoded_buffer_storage;
  absl::string_view decoded_buffer = buffer;
  if (!search_string.empty()) {
    decoded_buffer_storage = FindCommentMetadata(buffer, search_string);
    if (!decoded_buffer_storage) {
      return nullptr;
    }
    decoded_buffer = *decoded_buffer_storage;
  }
  if (!decoded_buffer_storage && filename.size() >= 2 &&
      filename.find(".h", filename.size() - 2) != std::string::npos) {
    decoded_buffer_storage = LoadHeaderMetadata(buffer);
    if (!decoded_buffer_storage) {
      LOG(WARNING) << filename << " wasn't a metadata header.";
    } else {
      decoded_buffer = *decoded_buffer_storage;
      modified_filename = filename.substr(0, filename.size() - 2);
    }
  }
  for (const auto& support : supports_) {
    if (auto metadata = support->ParseFile(filename, modified_filename,
                                           decoded_buffer, target_buffer)) {
      return metadata;
    }
  }
  return nullptr;
}

}  // namespace kythe
