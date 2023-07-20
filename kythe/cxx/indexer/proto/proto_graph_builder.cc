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

#include "kythe/cxx/indexer/proto/proto_graph_builder.h"

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "kythe/cxx/common/vname_ordering.h"
#include "kythe/cxx/indexer/proto/comments.h"

namespace kythe {

using ::kythe::proto::VName;

namespace {
// Pretty-prints a VName.
std::string StringifyNode(const VName& v_name) {
  return absl::StrCat(v_name.path(), ":", v_name.signature());
}

// Pretty-prints a NodeKindID.
std::string StringifyKind(NodeKindID kind) {
  return std::string(spelling_of(kind));
}

// Pretty-prints an EdgeKindID.
std::string StringifyKind(EdgeKindID kind) {
  return std::string(spelling_of(kind));
}
}  // anonymous namespace

void ProtoGraphBuilder::SetText(const VName& node_name,
                                const std::string& content) {
  DLOG(LEVEL(-1)) << "Setting text (length = " << content.length()
                  << ") for: " << StringifyNode(node_name);
  recorder_->AddProperty(VNameRef(node_name), kythe::PropertyID::kText,
                         content);
  current_file_contents_ = content;
}

void ProtoGraphBuilder::MaybeAddEdgeFromMetadata(const Location& location,
                                                 const VName& target) {
  if (meta_ == nullptr) {
    return;
  }
  auto rules = meta_->rules().equal_range(location.begin);
  for (auto rule = rules.first; rule != rules.second; ++rule) {
    if (location.end == rule->second.end) {
      EdgeKindID edge_kind;
      if (of_spelling(rule->second.edge_out, &edge_kind)) {
        VName source(rule->second.vname);
        source.set_corpus(target.corpus());
        if (rule->second.reverse_edge) {
          AddEdge(source, target, edge_kind);
        } else {
          AddEdge(target, source, edge_kind);
        }
      }
    }
  }
}

void ProtoGraphBuilder::MaybeAddMetadataFileRules(const VName& file) {
  if (meta_ == nullptr) {
    return;
  }
  for (const auto& rule : meta_->file_scope_rules()) {
    EdgeKindID edge_kind;
    if (of_spelling(rule.edge_out, &edge_kind)) {
      VName source(rule.vname);
      source.set_corpus(file.corpus());
      if (rule.reverse_edge) {
        AddEdge(source, file, edge_kind);
      } else {
        AddEdge(file, source, edge_kind);
      }
    }
  }
}

void ProtoGraphBuilder::AddNode(const VName& node_name, NodeKindID node_kind) {
  DLOG(LEVEL(-1)) << "Writing node: " << StringifyNode(node_name) << "["
                  << StringifyKind(node_kind) << "]";
  recorder_->AddProperty(VNameRef(node_name), node_kind);
}

void ProtoGraphBuilder::AddEdge(const VName& start, const VName& end,
                                EdgeKindID start_to_end_kind) {
  DLOG(LEVEL(-1)) << "Writing edge: " << StringifyNode(start) << " >-->--["
                  << StringifyKind(start_to_end_kind) << "]-->--> "
                  << StringifyNode(end);
  recorder_->AddEdge(VNameRef(start), start_to_end_kind, VNameRef(end));
}

VName ProtoGraphBuilder::CreateAndAddAnchorNode(const Location& location) {
  VName anchor = location.file;
  anchor.set_language(kLanguageName);

  auto* const signature = anchor.mutable_signature();
  absl::StrAppend(signature, "@", location.begin, ":", location.end);

  AddNode(anchor, NodeKindID::kAnchor);
  recorder_->AddProperty(VNameRef(anchor), PropertyID::kLocationStartOffset,
                         location.begin);
  recorder_->AddProperty(VNameRef(anchor), PropertyID::kLocationEndOffset,
                         location.end);

  return anchor;
}

VName ProtoGraphBuilder::CreateAndAddDocNode(const Location& location,
                                             const VName& element) {
  VName doc = location.file;
  doc.set_language(kLanguageName);
  doc.set_signature(
      absl::StrCat("doc-", location.begin, "-", element.signature()));

  // Adjust the text to splice out comment markers, as per
  // http://www.kythe.io/docs/schema/#doc
  AddNode(doc, NodeKindID::kDoc);
  std::string comment = StripCommentMarkers(current_file_contents_.substr(
      location.begin, location.end - location.begin));
  recorder_->AddProperty(VNameRef(doc), PropertyID::kText, comment);
  return doc;
}

void ProtoGraphBuilder::AddImport(const std::string& import,
                                  const Location& location) {
  VName dep = vname_for_rel_path_(import);
  LOG(INFO) << "DEPENDENCY : " << URI(dep).ToString();
  VName anchor = CreateAndAddAnchorNode(location);
  AddEdge(anchor, dep, EdgeKindID::kRefIncludes);
}

void ProtoGraphBuilder::AddNamespace(const VName& package,
                                     const Location& location) {
  VName anchor = CreateAndAddAnchorNode(location);
  AddNode(package, NodeKindID::kPackage);
  AddEdge(anchor, package, EdgeKindID::kRef);
}

void ProtoGraphBuilder::AddValueToEnum(const VName& enum_type,
                                       const VName& value,
                                       const Location& location) {
  VName anchor = CreateAndAddAnchorNode(location);
  AddNode(value, NodeKindID::kVariable);
  AddEdge(anchor, value, EdgeKindID::kDefinesBinding);
  MaybeAddEdgeFromMetadata(location, value);
  AddEdge(value, enum_type, EdgeKindID::kChildOf);
}

void ProtoGraphBuilder::AddFieldToMessage(const VName* parent,
                                          const VName& message,
                                          const VName* oneof,
                                          const VName& field,
                                          const Location& location) {
  VName anchor = CreateAndAddAnchorNode(location);
  AddNode(field, NodeKindID::kVariable);
  recorder_->AddProperty(VNameRef(field), PropertyID::kSubkind, "field");
  AddEdge(anchor, field, EdgeKindID::kDefinesBinding);
  MaybeAddEdgeFromMetadata(location, field);
  if (parent != nullptr) {
    AddEdge(field, *parent, EdgeKindID::kChildOf);
  }
  if (parent == nullptr || !VNameEquals(message, *parent)) {
    // Extension; add an edge to the message being extended.
    AddEdge(field, message, EdgeKindID::kExtends);
  }
  if (oneof != nullptr) {
    AddEdge(field, *oneof, EdgeKindID::kChildOf);
  }
}

void ProtoGraphBuilder::AddOneofToMessage(const VName& message,
                                          const VName& oneof,
                                          const Location& location) {
  VName anchor = CreateAndAddAnchorNode(location);
  AddNode(oneof, NodeKindID::kSum);
  AddEdge(anchor, oneof, EdgeKindID::kDefinesBinding);
  MaybeAddEdgeFromMetadata(location, oneof);
  AddEdge(oneof, message, EdgeKindID::kChildOf);
}

void ProtoGraphBuilder::AddMethodToService(const VName& service,
                                           const VName& method,
                                           const Location& location) {
  VName anchor = CreateAndAddAnchorNode(location);
  AddNode(method, NodeKindID::kFunction);
  AddEdge(anchor, method, EdgeKindID::kDefinesBinding);
  MaybeAddEdgeFromMetadata(location, method);
  AddEdge(method, service, EdgeKindID::kChildOf);
}

void ProtoGraphBuilder::AddEnumType(const VName* parent, const VName& enum_type,
                                    const Location& location) {
  VName anchor = CreateAndAddAnchorNode(location);
  AddNode(enum_type, NodeKindID::kSum);
  AddEdge(anchor, enum_type, EdgeKindID::kDefinesBinding);
  MaybeAddEdgeFromMetadata(location, enum_type);
  if (parent != nullptr) {
    AddEdge(enum_type, *parent, EdgeKindID::kChildOf);
  }
}

void ProtoGraphBuilder::AddMessageType(const VName* parent,
                                       const VName& message,
                                       const Location& location) {
  VName anchor = CreateAndAddAnchorNode(location);
  AddNode(message, NodeKindID::kRecord);
  AddEdge(anchor, message, EdgeKindID::kDefinesBinding);
  MaybeAddEdgeFromMetadata(location, message);
  if (parent != nullptr) {
    AddEdge(message, *parent, EdgeKindID::kChildOf);
  }
}

void ProtoGraphBuilder::AddReference(const VName& referent,
                                     const Location& location) {
  VName anchor = CreateAndAddAnchorNode(location);
  AddEdge(anchor, referent, EdgeKindID::kRef);
}

void ProtoGraphBuilder::AddTyping(const VName& term, const VName& type) {
  AddEdge(term, type, EdgeKindID::kHasType);
}

void ProtoGraphBuilder::AddService(const VName* parent, const VName& service,
                                   const Location& location) {
  VName anchor = CreateAndAddAnchorNode(location);
  AddNode(service, NodeKindID::kInterface);
  AddEdge(anchor, service, EdgeKindID::kDefinesBinding);
  MaybeAddEdgeFromMetadata(location, service);
  if (parent != nullptr) {
    AddEdge(service, *parent, EdgeKindID::kChildOf);
  }
}

void ProtoGraphBuilder::AddDocComment(const VName& element,
                                      const Location& location) {
  VName doc = CreateAndAddDocNode(location, element);
  AddEdge(doc, element, EdgeKindID::kDocuments);
}

void ProtoGraphBuilder::AddCodeFact(const VName& element,
                                    const MarkedSource& code) {
  recorder_->AddMarkedSource(VNameRef(element), code);
}

void ProtoGraphBuilder::SetDeprecated(const VName& node_name) {
  DLOG(LEVEL(-1)) << "Adding deprecation tag for " << StringifyNode(node_name);
  recorder_->AddProperty(VNameRef(node_name), kythe::PropertyID::kTagDeprecated,
                         "");
}

}  // namespace kythe
