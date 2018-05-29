/*
 * Copyright 2015 Google Inc. All rights reserved.
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

#include "glog/logging.h"
#include "kythe/cxx/common/json_proto.h"  // DecodeBase64
#include "kythe/cxx/common/proto_conversions.h"
#include "kythe/cxx/common/schema/edges.h"
#include "kythe/proto/storage.pb.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

namespace kythe {

#define JSON_SAFE_LOAD(key, type)                                         \
  const auto key##_pair = value.FindMember(#key);                         \
  if (key##_pair == value.MemberEnd() || !key##_pair->value.Is##type()) { \
    LOG(WARNING) << "Unexpected or missing key " #key " : " #type;        \
    return false;                                                         \
  }                                                                       \
  const auto &key = key##_pair->value;

#define JSON_SAFE_LOAD_STRING(dest, key)                                 \
  const auto key##_pair = value.FindMember(#key);                        \
  if (key##_pair != value.MemberEnd() && key##_pair->value.IsString()) { \
    dest = key##_pair->value.GetString();                                \
  }

namespace {
bool LoadVName(const rapidjson::Value &value, proto::VName *vname_out) {
  JSON_SAFE_LOAD_STRING(*(vname_out->mutable_signature()), signature);
  JSON_SAFE_LOAD_STRING(*(vname_out->mutable_root()), root);
  JSON_SAFE_LOAD_STRING(*(vname_out->mutable_path()), path);
  JSON_SAFE_LOAD_STRING(*(vname_out->mutable_language()), language);
  JSON_SAFE_LOAD_STRING(*(vname_out->mutable_corpus()), corpus);
  if (vname_out->corpus().empty() && vname_out->path().empty() &&
      vname_out->root().empty() && vname_out->signature().empty() &&
      vname_out->language().empty()) {
    LOG(WARNING) << "When loading metadata: empty vname.";
    return false;
  }
  return true;
}

std::unique_ptr<llvm::MemoryBuffer> LoadCommentMetadata(
    llvm::StringRef buf_string, size_t comment_slash_pos,
    size_t data_start_pos) {
  google::protobuf::string raw_data;
  // Over-reserves--though we expect the comment to be the only thing in the
  // file or the last thing in the file, so this approximation is reasonable.
  raw_data.reserve(buf_string.size() - comment_slash_pos);
  size_t pos = data_start_pos;
  // Tolerate single-line comments as well as multi-line comments.
  // If there's a single-line comment, it should be the only thing in the
  // file.
  bool single_line = buf_string[comment_slash_pos + 1] == '/';
  auto next_term =
      single_line ? llvm::StringRef::npos : buf_string.find("*/", pos);
  for (; pos < buf_string.size();) {
    while (pos < buf_string.size() && isspace(buf_string[pos])) ++pos;
    auto next_newline = buf_string.find("\n", pos);
    if (next_term != llvm::StringRef::npos &&
        (next_newline == llvm::StringRef::npos || next_term < next_newline)) {
      raw_data.append(buf_string.data() + pos, next_term - pos);
    } else if (next_newline != llvm::StringRef::npos) {
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
  google::protobuf::string decoded;
  return DecodeBase64(raw_data, &decoded)
             ? llvm::MemoryBuffer::getMemBufferCopy(ToStringRef(decoded))
             : nullptr;
}

/// \brief Attempts to load buffer as a header-style metadata file.
/// \param buffer data to try and parse.
/// \return the decoded metadata on success or null on failure.
std::unique_ptr<llvm::MemoryBuffer> LoadHeaderMetadata(
    const llvm::MemoryBuffer *buffer) {
  if (buffer->getBufferSize() < 2) {
    return nullptr;
  }
  auto buf_string = buffer->getBuffer();
  if (buf_string[0] != '/' || !(buf_string[1] == '*' || buf_string[1] == '/')) {
    return nullptr;
  }
  return LoadCommentMetadata(buf_string, 0, 2);
}

/// \brief Attempts to load buffer as an inline metadata file
/// \param buffer data to try and parse.
/// \param search_string the string identifying the data.
/// \return the decoded metadata on success or null on failure.
std::unique_ptr<llvm::MemoryBuffer> FindCommentMetadata(
    const llvm::MemoryBuffer *buffer, const std::string &search_string) {
  auto buf_string = buffer->getBuffer();
  auto comment_start = buf_string.find("/* " + search_string);
  if (comment_start == llvm::StringRef::npos) {
    comment_start = buf_string.find("// " + search_string);
    if (comment_start == llvm::StringRef::npos) {
      return nullptr;
    }
  }
  return LoadCommentMetadata(buf_string, comment_start,
                             comment_start + 3 + search_string.size());
}
}  // anonymous namespace

bool KytheMetadataSupport::LoadMetaElement(const rapidjson::Value &value,
                                           MetadataFile::Rule *rule) {
  JSON_SAFE_LOAD(type, String);
  if (type == "nop") {
    return true;
  }
  JSON_SAFE_LOAD(edge, String);
  llvm::StringRef edge_string = edge.GetString();
  if (edge_string.empty()) {
    LOG(WARNING) << "When loading metadata: empty edge.";
    return false;
  }
  bool reverse_edge = false;
  if (edge_string[0] == '%') {
    edge_string = edge_string.drop_front(1);
    reverse_edge = true;
  }
  if (type == "anchor_defines") {
    JSON_SAFE_LOAD(begin, Number);
    JSON_SAFE_LOAD(end, Number);
    JSON_SAFE_LOAD(vname, Object);
    proto::VName vname_out;
    if (!LoadVName(vname, &vname_out)) {
      return false;
    }
    if (!begin.IsUint() || !end.IsUint()) {
      return false;
    }
    unsigned begin_int = begin.GetUint();
    unsigned end_int = end.GetUint();
    *rule = MetadataFile::Rule{
        begin_int,   end_int,   kythe::common::schema::kDefinesBinding,
        edge_string, vname_out, reverse_edge,
        false,       0,         0};
    return true;
  } else if (type == "anchor_anchor") {
    JSON_SAFE_LOAD(source_begin, Number);
    JSON_SAFE_LOAD(source_end, Number);
    JSON_SAFE_LOAD(target_begin, Number);
    JSON_SAFE_LOAD(target_end, Number);
    JSON_SAFE_LOAD(source_vname, Object);
    proto::VName vname_out;
    if (!LoadVName(source_vname, &vname_out)) {
      return false;
    }
    if (!source_begin.IsUint() || !source_end.IsUint() ||
        !target_begin.IsUint() || !target_end.IsUint()) {
      return false;
    }
    unsigned source_begin_int = source_begin.GetUint();
    unsigned source_end_int = source_end.GetUint();
    unsigned target_begin_int = target_begin.GetUint();
    unsigned target_end_int = target_end.GetUint();
    *rule = MetadataFile::Rule{target_begin_int,
                               target_end_int,
                               kythe::common::schema::kDefinesBinding,
                               edge_string,
                               vname_out,
                               !reverse_edge,
                               true,
                               source_begin_int,
                               source_end_int};
    return true;
  } else {
    LOG(WARNING) << "When loading metadata: unknown meta type.";
    return false;
  }
}

#undef JSON_SAFE_LOAD
#undef JSON_SAFE_LOAD_STRING

std::unique_ptr<MetadataFile> KytheMetadataSupport::LoadFromJSON(
    llvm::StringRef json) {
  rapidjson::Document document;
  document.Parse(json.str().c_str());
  if (document.HasParseError()) {
    LOG(WARNING) << rapidjson::GetParseError_En(document.GetParseError())
                 << " near offset " << document.GetErrorOffset();
    return nullptr;
  }
  if (!document.IsObject()) {
    LOG(WARNING)
        << "When loading metadata: root element in JSON was not an object.";
    return nullptr;
  }
  const auto doc_type = document.FindMember("type");
  if (doc_type == document.MemberEnd() || !doc_type->value.IsString()) {
    LOG(WARNING) << "When loading metadata: JSON element is missing type.";
    return nullptr;
  }
  const auto doc_type_val = llvm::StringRef(doc_type->value.GetString());
  if (doc_type_val != "kythe0") {
    LOG(WARNING) << "When loading metadata: JSON element has unexpected type "
                 << doc_type_val.str();
    return nullptr;
  }
  const auto meta = document.FindMember("meta");
  if (meta == document.MemberEnd() || !meta->value.IsArray()) {
    LOG(WARNING)
        << "When loading metadata: kythe0.meta missing or not an array";
    return nullptr;
  }
  std::vector<MetadataFile::Rule> rules;
  for (rapidjson::Value::ConstValueIterator meta_element = meta->value.Begin();
       meta_element != meta->value.End(); ++meta_element) {
    if (!meta_element->IsObject()) {
      LOG(WARNING) << "When loading metadata: kythe0.meta[i] not an object";
      return nullptr;
    }
    MetadataFile::Rule rule;
    if (!LoadMetaElement(*meta_element, &rule)) {
      return nullptr;
    }
    rules.push_back(rule);
  }
  return MetadataFile::LoadFromRules(rules.begin(), rules.end());
}

std::unique_ptr<kythe::MetadataFile> KytheMetadataSupport::ParseFile(
    const std::string &raw_filename, const std::string &filename,
    const llvm::MemoryBuffer *buffer) {
  auto metadata = LoadFromJSON(buffer->getBuffer());
  if (!metadata) {
    LOG(WARNING) << "Failed loading " << raw_filename;
  }
  return metadata;
}

void MetadataSupports::UseVNameLookup(VNameLookup lookup) const {
  for (auto &support : supports_) {
    support->UseVNameLookup(lookup);
  }
}

std::unique_ptr<kythe::MetadataFile> MetadataSupports::ParseFile(
    const std::string &filename, const llvm::MemoryBuffer *buffer,
    const std::string &search_string) const {
  std::string modified_filename = filename;
  std::unique_ptr<llvm::MemoryBuffer> decoded_buffer_storage;
  const llvm::MemoryBuffer *decoded_buffer = buffer;
  if (!search_string.empty()) {
    decoded_buffer_storage = FindCommentMetadata(buffer, search_string);
    if (decoded_buffer_storage == nullptr) {
      return nullptr;
    }
    decoded_buffer = decoded_buffer_storage.get();
  }
  if (decoded_buffer_storage == nullptr && filename.size() >= 2 &&
      filename.find(".h", filename.size() - 2) != std::string::npos) {
    decoded_buffer_storage = LoadHeaderMetadata(buffer);
    if (decoded_buffer_storage != nullptr) {
      decoded_buffer = decoded_buffer_storage.get();
      modified_filename = filename.substr(0, filename.size() - 2);
    } else {
      LOG(WARNING) << filename << " wasn't a metadata header.";
    }
  }
  for (const auto &support : supports_) {
    if (auto metadata =
            support->ParseFile(filename, modified_filename, decoded_buffer)) {
      return metadata;
    }
  }
  return nullptr;
}

}  // namespace kythe
