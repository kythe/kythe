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

#include "kythe_metadata_file.h"

#include "kythe/proto/storage.pb.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

namespace kythe {

#define JSON_SAFE_LOAD(key, type)                                         \
  const auto key##_pair = value.FindMember(#key);                         \
  if (key##_pair == value.MemberEnd() || !key##_pair->value.Is##type()) { \
    *error_text = "Unexpected or missing key " #key " : " #type;          \
    return false;                                                         \
  }                                                                       \
  const auto &key = key##_pair->value;

#define JSON_SAFE_LOAD_STRING(dest, key)                                 \
  const auto key##_pair = value.FindMember(#key);                        \
  if (key##_pair != value.MemberEnd() && key##_pair->value.IsString()) { \
    dest = key##_pair->value.GetString();                                \
  }

static bool LoadVName(const rapidjson::Value &value, proto::VName *vname_out,
                      std::string *error_text) {
  JSON_SAFE_LOAD_STRING(*(vname_out->mutable_signature()), signature);
  JSON_SAFE_LOAD_STRING(*(vname_out->mutable_root()), root);
  JSON_SAFE_LOAD_STRING(*(vname_out->mutable_path()), path);
  JSON_SAFE_LOAD_STRING(*(vname_out->mutable_language()), language);
  JSON_SAFE_LOAD_STRING(*(vname_out->mutable_corpus()), corpus);
  if (vname_out->corpus().empty() && vname_out->path().empty() &&
      vname_out->root().empty() && vname_out->signature().empty() &&
      vname_out->language().empty()) {
    *error_text = "Empty vname.";
    return false;
  }
  return true;
}

bool MetadataFile::LoadMetaElement(const rapidjson::Value &value,
                                   std::string *error_text) {
  JSON_SAFE_LOAD(type, String);
  if (type == "nop") {
    return true;
  }
  if (type != "anchor_defines") {
    *error_text = "Unknown meta type.";
    return false;
  }
  JSON_SAFE_LOAD(begin, Number);
  JSON_SAFE_LOAD(end, Number);
  JSON_SAFE_LOAD(edge, String);
  JSON_SAFE_LOAD(vname, Object);
  proto::VName vname_out;
  if (!LoadVName(vname, &vname_out, error_text)) {
    return false;
  }
  if (!begin.IsUint() || !end.IsUint()) {
    return false;
  }
  unsigned begin_int = begin.GetUint();
  unsigned end_int = end.GetUint();
  llvm::StringRef edge_string = edge.GetString();
  if (edge_string.empty()) {
    *error_text = "Empty edge.";
    return false;
  }
  bool reverse_edge = false;
  if (edge_string[0] == '%') {
    edge_string = edge_string.drop_front(1);
    reverse_edge = true;
  }

  rules_.emplace(begin_int, Rule{begin_int, end_int, "/kythe/edge/defines",
                                 edge_string, vname_out, reverse_edge});

  return true;
}

#undef JSON_SAFE_LOAD
#undef JSON_SAFE_LOAD_STRING

std::unique_ptr<MetadataFile> MetadataFile::LoadFromJSON(
    llvm::StringRef json, std::string *error_text) {
  assert(error_text != nullptr);
  rapidjson::Document document;
  document.Parse(json.str().c_str());
  if (document.HasParseError()) {
    if (error_text) {
      *error_text = rapidjson::GetParseError_En(document.GetParseError());
      error_text->append(" near offset ");
      error_text->append(std::to_string(document.GetErrorOffset()));
    }
    return nullptr;
  }
  if (!document.IsObject()) {
    *error_text = "Root element in JSON was not an object.";
    return nullptr;
  }
  const auto doc_type = document.FindMember("type");
  if (doc_type == document.MemberEnd() || !doc_type->value.IsString()) {
    *error_text = "JSON element is missing type.";
    return nullptr;
  }
  const auto doc_type_val = llvm::StringRef(doc_type->value.GetString());
  if (doc_type_val != "kythe0") {
    *error_text = "JSON element has unexpected type ";
    error_text->append(doc_type_val);
    return nullptr;
  }
  const auto meta = document.FindMember("meta");
  if (meta == document.MemberEnd() || !meta->value.IsArray()) {
    *error_text = "kythe0.meta missing or not an array";
    return nullptr;
  }
  std::unique_ptr<MetadataFile> meta_file(new MetadataFile());
  for (rapidjson::Value::ConstValueIterator meta_element = meta->value.Begin();
       meta_element != meta->value.End(); ++meta_element) {
    if (!meta_element->IsObject()) {
      *error_text = "kythe0.meta[i] not an object";
      return nullptr;
    }
    if (!meta_file->LoadMetaElement(*meta_element, error_text)) {
      return nullptr;
    }
  }
  return meta_file;
}

std::unique_ptr<kythe::MetadataFile> KytheMetadataSupport::ParseFile(
    const std::string &filename, const llvm::MemoryBuffer *buffer) {
  std::string error;
  auto metadata = MetadataFile::LoadFromJSON(buffer->getBuffer(), &error);
  if (!metadata) {
    fprintf(stderr, "Couldn't load %s: %s\n", filename.c_str(), error.c_str());
  }
  return metadata;
}

}  // namespace kythe
