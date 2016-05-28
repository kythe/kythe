/*
 * Copyright 2014 Google Inc. All rights reserved.
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

#include "KytheGraphRecorder.h"

#include "kythe/proto/storage.pb.h"

namespace kythe {

static const std::string *const kNodeKindSpellings[] = {
    new std::string("anchor"),   new std::string("file"),
    new std::string("variable"), new std::string("talias"),
    new std::string("tapp"),     new std::string("tnominal"),
    new std::string("record"),   new std::string("sum"),
    new std::string("constant"), new std::string("abs"),
    new std::string("absvar"),   new std::string("name"),
    new std::string("function"), new std::string("lookup"),
    new std::string("macro"),    new std::string("interface"),
    new std::string("package"),  new std::string("tsigma"),
    new std::string("doc"),      new std::string("builtin"),
    new std::string("meta")};

static const std::string *kEdgeKindSpellings[] = {
    new std::string("/kythe/edge/defines"),
    new std::string("/kythe/edge/named"),
    new std::string("/kythe/edge/typed"),
    new std::string("/kythe/edge/ref"),
    new std::string("/kythe/edge/param"),
    new std::string("/kythe/edge/aliases"),
    new std::string("/kythe/edge/completes/uniquely"),
    new std::string("/kythe/edge/completes"),
    new std::string("/kythe/edge/childof"),
    new std::string("/kythe/edge/specializes"),
    new std::string("/kythe/edge/ref/call"),
    new std::string("/kythe/edge/ref/expands"),
    new std::string("/kythe/edge/undefines"),
    new std::string("/kythe/edge/ref/includes"),
    new std::string("/kythe/edge/ref/queries"),
    new std::string("/kythe/edge/instantiates"),
    new std::string("/kythe/edge/ref/expands/transitive"),
    new std::string("/kythe/edge/extends/public"),
    new std::string("/kythe/edge/extends/protected"),
    new std::string("/kythe/edge/extends/private"),
    new std::string("/kythe/edge/extends"),
    new std::string("/kythe/edge/extends/public/virtual"),
    new std::string("/kythe/edge/extends/protected/virtual"),
    new std::string("/kythe/edge/extends/private/virtual"),
    new std::string("/kythe/edge/extends/virtual"),
    new std::string("/kythe/edge/specializes/speculative"),
    new std::string("/kythe/edge/instantiates/speculative"),
    new std::string("/kythe/edge/documents"),
    new std::string("/kythe/edge/ref/doc"),
    new std::string("/kythe/edge/generates"),
    new std::string("/kythe/edge/defines/binding"),
    new std::string("/kythe/edge/overrides"),
    new std::string("/kythe/edge/childof/context")};

bool of_spelling(llvm::StringRef str, EdgeKindID *edge_id) {
  size_t edge_index = 0;
  for (auto *edge : kEdgeKindSpellings) {
    if (*edge == str) {
      *edge_id = static_cast<kythe::EdgeKindID>(edge_index);
      return true;
    }
    ++edge_index;
  }
  return false;
}

static const std::string *const kPropertySpellings[] = {
    new std::string("/kythe/loc"),
    new std::string("/kythe/loc/uri"),
    new std::string("/kythe/loc/start"),
    new std::string("/kythe/loc/start/row"),
    new std::string("/kythe/loc/start"),
    new std::string("/kythe/loc/end"),
    new std::string("/kythe/loc/end/row"),
    new std::string("/kythe/loc/end"),
    new std::string("/kythe/text"),
    new std::string("/kythe/complete"),
    new std::string("/kythe/subkind"),
    new std::string("/kythe/node/kind"),
    new std::string("/kythe/format")};

static const std::string *const kEmptyStringSpelling = new std::string("");

static const std::string *const kRootPropertySpelling = new std::string("/");

llvm::StringRef spelling_of(PropertyID property_id) {
  const auto *str = kPropertySpellings[static_cast<ptrdiff_t>(property_id)];
  return llvm::StringRef(str->data(), str->size());
}

llvm::StringRef spelling_of(NodeKindID node_kind_id) {
  const auto *str = kNodeKindSpellings[static_cast<ptrdiff_t>(node_kind_id)];
  return llvm::StringRef(str->data(), str->size());
}

llvm::StringRef spelling_of(EdgeKindID edge_kind_id) {
  const auto *str = kEdgeKindSpellings[static_cast<ptrdiff_t>(edge_kind_id)];
  return llvm::StringRef(str->data(), str->size());
}

void KytheGraphRecorder::AddProperty(const VNameRef &node_vname,
                                     PropertyID property_id,
                                     const std::string &property_value) {
  stream_->Emit(
      FactRef{&node_vname, spelling_of(property_id),
              llvm::StringRef(property_value.data(), property_value.size())});
}

void KytheGraphRecorder::AddProperty(const VNameRef &node_vname,
                                     PropertyID property_id,
                                     const size_t property_value) {
  AddProperty(node_vname, property_id, std::to_string(property_value));
}

void KytheGraphRecorder::AddEdge(const VNameRef &edge_from,
                                 EdgeKindID edge_kind_id,
                                 const VNameRef &edge_to) {
  stream_->Emit(EdgeRef{&edge_from, spelling_of(edge_kind_id), &edge_to});
}

void KytheGraphRecorder::AddEdge(const VNameRef &edge_from,
                                 EdgeKindID edge_kind_id,
                                 const VNameRef &edge_to, uint32_t ordinal) {
  stream_->Emit(
      OrdinalEdgeRef{&edge_from, spelling_of(edge_kind_id), &edge_to, ordinal});
}

void KytheGraphRecorder::AddFileContent(const VNameRef &file_vname,
                                        const llvm::StringRef &file_content) {
  AddProperty(file_vname, PropertyID::kNodeKind,
              spelling_of(NodeKindID::kFile));
  AddProperty(file_vname, PropertyID::kText, file_content.str());
}

}  // namespace kythe
