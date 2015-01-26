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

#include "KytheGraphObserver.h"

#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclFriend.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclOpenMP.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/ExprObjC.h"
#include "clang/AST/NestedNameSpecifier.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/StmtCXX.h"
#include "clang/AST/StmtObjC.h"
#include "clang/AST/StmtOpenMP.h"
#include "clang/AST/TemplateBase.h"
#include "clang/AST/TemplateName.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/SourceManager.h"

#include "IndexerASTHooks.h"
#include "KytheGraphRecorder.h"

namespace kythe {

using clang::SourceLocation;
using kythe::proto::Entry;
using llvm::StringRef;

static const char *CompletenessToString(
    KytheGraphObserver::Completeness completeness) {
  switch (completeness) {
    case KytheGraphObserver::Completeness::Definition:
      return "definition";
    case KytheGraphObserver::Completeness::Complete:
      return "complete";
    case KytheGraphObserver::Completeness::Incomplete:
      return "incomplete";
  }
  assert(0 && "Invalid enumerator passed to CompletenessToString.");
  return "invalid-completeness";
}

kythe::proto::VName KytheGraphObserver::VNameFromFileEntry(
    const clang::FileEntry *file_entry) {
  auto lookup_result = path_to_vname_.find(file_entry->getName());
  if (lookup_result != path_to_vname_.end()) {
    return lookup_result->second;
  }
  kythe::proto::VName out_name;
  out_name.set_language("c++");
  out_name.set_path(file_entry->getName());
  if (!default_root_.empty()) {
    out_name.set_root(default_root_);
  }
  if (!default_corpus_.empty()) {
    out_name.set_corpus(default_corpus_);
  }
  return out_name;
}

kythe::proto::VName KytheGraphObserver::VNameFromFileID(
    const clang::FileID &file_id) {
  const clang::FileEntry *file_entry =
      SourceManager->getFileEntryForID(file_id);
  if (file_entry) {
    auto lookup_result = path_to_vname_.find(file_entry->getName());
    if (lookup_result != path_to_vname_.end()) {
      return lookup_result->second;
    }
  }
  kythe::proto::VName out_name;
  out_name.set_language("c++");
  if (file_entry) {
    out_name.set_path(file_entry->getName());
  } else {
    // TODO(zarko): What should we do in this case? We could invent a name
    // for file_id (maybe just stringify file_id + some salt) to keep from
    // breaking other parts of the index.
    out_name.set_path("(invalid)");
  }
  if (!default_root_.empty()) {
    out_name.set_root(default_root_);
  }
  if (!default_corpus_.empty()) {
    out_name.set_corpus(default_corpus_);
  }
  return out_name;
}

kythe::proto::VName KytheGraphObserver::VNameFromRange(
    const GraphObserver::Range &range) {
  const clang::SourceRange &source_range = range.PhysicalRange;
  kythe::proto::VName out_name;
  out_name.set_language("c++");
  clang::SourceLocation begin = source_range.getBegin();
  clang::SourceLocation end = source_range.getEnd();
  assert(begin.isValid());
  assert(end.isValid());
  if (begin.isMacroID()) {
    begin = SourceManager->getExpansionLoc(begin);
  }
  if (end.isMacroID()) {
    end = SourceManager->getExpansionLoc(end);
  }
  kythe::proto::VName file_name(
      VNameFromFileID(SourceManager->getFileID(begin)));
  size_t begin_offset = SourceManager->getFileOffset(begin);
  size_t end_offset = SourceManager->getFileOffset(end);
  auto *const signature = out_name.mutable_signature();
  signature->append("@");
  signature->append(std::to_string(begin_offset));
  signature->append(":");
  signature->append(std::to_string(end_offset));
  signature->append("@");
  signature->append(file_name.path());
  if (range.Kind == GraphObserver::Range::RangeKind::Wraith) {
    signature->append("@");
    signature->append(range.Context.ToString());
  }
  if (!default_root_.empty()) {
    out_name.set_root(default_root_);
  }
  if (!default_corpus_.empty()) {
    out_name.set_corpus(default_corpus_);
  }
  return out_name;
}

void KytheGraphObserver::RecordSourceLocation(
    clang::SourceLocation source_location, PropertyID offset_id) {
  if (source_location.isMacroID()) {
    source_location = SourceManager->getExpansionLoc(source_location);
  }
  size_t offset = SourceManager->getFileOffset(source_location);
  recorder_->AddProperty(offset_id, offset);
}

void KytheGraphObserver::recordMacroNode(const NodeId &macro_id) {
  recorder_->BeginNode(VNameFromNodeId(macro_id), NodeKindID::kMacro);
  recorder_->EndNode();
}

void KytheGraphObserver::recordExpandsRange(const Range &source_range,
                                            const NodeId &macro_id) {
  RecordAnchor(source_range, VNameFromNodeId(macro_id),
               EdgeKindID::kRefExpands);
}

void KytheGraphObserver::recordUndefinesRange(const Range &source_range,
                                              const NodeId &macro_id) {
  RecordAnchor(source_range, VNameFromNodeId(macro_id), EdgeKindID::kUndefines);
}

void KytheGraphObserver::recordBoundQueryRange(const Range &source_range,
                                               const NodeId &macro_id) {
  RecordAnchor(source_range, VNameFromNodeId(macro_id),
               EdgeKindID::kRefQueries);
}

void KytheGraphObserver::recordUnboundQueryRange(const Range &source_range,
                                                 const NameId &macro_name) {
  RecordAnchor(source_range, RecordName(macro_name), EdgeKindID::kRefQueries);
}

void KytheGraphObserver::recordIncludesRange(const Range &source_range,
                                             const clang::FileEntry *File) {
  RecordAnchor(source_range, VNameFromFileEntry(File),
               EdgeKindID::kRefIncludes);
}

void KytheGraphObserver::recordVariableNode(const NameId &name,
                                            const NodeId &node,
                                            Completeness completeness) {
  KytheGraphRecorder::VName name_vname(RecordName(name));
  KytheGraphRecorder::VName node_vname(VNameFromNodeId(node));
  recorder_->BeginNode(node_vname, NodeKindID::kVariable);
  recorder_->AddProperty(PropertyID::kComplete,
                         CompletenessToString(completeness));
  recorder_->EndNode();
  recorder_->AddEdge(node_vname, EdgeKindID::kNamed, name_vname);
}

void KytheGraphObserver::RecordDeferredNodes() {
  for (const auto &range : deferred_anchors_) {
    KytheGraphRecorder::VName anchor_name(VNameFromRange(range));
    KytheGraphRecorder::VName file_name(VNameFromFileID(
        SourceManager->getFileID(range.PhysicalRange.getBegin())));
    recorder_->BeginNode(anchor_name, NodeKindID::kAnchor);
    RecordSourceLocation(range.PhysicalRange.getBegin(),
                         PropertyID::kLocationStartOffset);
    RecordSourceLocation(range.PhysicalRange.getEnd(),
                         PropertyID::kLocationEndOffset);
    recorder_->EndNode();
    recorder_->AddEdge(anchor_name, EdgeKindID::kChildOf, file_name);
    if (range.Kind == GraphObserver::Range::RangeKind::Wraith) {
      recorder_->AddEdge(anchor_name, EdgeKindID::kChildOf,
                         VNameFromNodeId(range.Context));
    }
  }
  deferred_anchors_.clear();
}

KytheGraphRecorder::VName KytheGraphObserver::RecordAnchor(
    const GraphObserver::Range &source_range,
    const kythe::proto::VName &primary_anchored_to,
    EdgeKindID anchor_edge_kind) {
  deferred_anchors_.insert(source_range);
  KytheGraphRecorder::VName anchor_name(VNameFromRange(source_range));
  recorder_->AddEdge(anchor_name, anchor_edge_kind, primary_anchored_to);
  return anchor_name;
}

void KytheGraphObserver::recordCallEdge(
    const GraphObserver::Range &source_range, const NodeId &caller_id,
    const NodeId &callee_id) {
  KytheGraphRecorder::VName anchor_name(RecordAnchor(
      source_range, VNameFromNodeId(caller_id), EdgeKindID::kChildOf));
  recorder_->AddEdge(anchor_name, EdgeKindID::kRefCall,
                     VNameFromNodeId(callee_id));
}

kythe::proto::VName KytheGraphObserver::VNameFromNodeId(
    const GraphObserver::NodeId &node_id) {
  KytheGraphRecorder::VName out_vname;
  out_vname.set_language("c++");
  if (!default_root_.empty()) {
    out_vname.set_root(default_root_);
  }
  if (!default_corpus_.empty()) {
    out_vname.set_corpus(default_corpus_);
  }
  out_vname.set_signature(node_id.ToString());
  return out_vname;
}

kythe::proto::VName KytheGraphObserver::RecordName(
    const GraphObserver::NameId &name_id) {
  KytheGraphRecorder::VName out_vname;
  out_vname.set_language("c++");
  if (!default_root_.empty()) {
    out_vname.set_root(default_root_);
  }
  if (!default_corpus_.empty()) {
    out_vname.set_corpus(default_corpus_);
  }
  const std::string name_id_string = name_id.ToString();
  out_vname.set_signature(name_id_string);
  if (written_name_ids_.insert(name_id_string).second) {
    recorder_->BeginNode(out_vname, NodeKindID::kName);
    recorder_->EndNode();
  }
  return out_vname;
}

void KytheGraphObserver::recordParamEdge(const NodeId &param_of_id,
                                         uint32_t ordinal,
                                         const NodeId &param_id) {
  recorder_->AddEdge(VNameFromNodeId(param_of_id), EdgeKindID::kParam,
                     VNameFromNodeId(param_id), ordinal);
}

void KytheGraphObserver::recordChildOfEdge(const NodeId &child_id,
                                           const NodeId &parent_id) {
  recorder_->AddEdge(VNameFromNodeId(child_id), EdgeKindID::kChildOf,
                     VNameFromNodeId(parent_id));
}

void KytheGraphObserver::recordTypeEdge(const NodeId &term_id,
                                        const NodeId &type_id) {
  recorder_->AddEdge(VNameFromNodeId(term_id), EdgeKindID::kHasType,
                     VNameFromNodeId(type_id));
}

void KytheGraphObserver::recordCallableAsEdge(const NodeId &from_id,
                                              const NodeId &to_id) {
  recorder_->AddEdge(VNameFromNodeId(from_id), EdgeKindID::kCallableAs,
                     VNameFromNodeId(to_id));
}

void KytheGraphObserver::recordSpecEdge(const NodeId &term_id,
                                        const NodeId &type_id) {
  recorder_->AddEdge(VNameFromNodeId(term_id), EdgeKindID::kSpecializes,
                     VNameFromNodeId(type_id));
}

void KytheGraphObserver::recordInstEdge(const NodeId &term_id,
                                        const NodeId &type_id) {
  recorder_->AddEdge(VNameFromNodeId(term_id), EdgeKindID::kInstantiates,
                     VNameFromNodeId(type_id));
}

GraphObserver::NodeId KytheGraphObserver::nodeIdForTypeAliasNode(
    const NameId &alias_name, const NodeId &aliased_type) {
  NodeId id_out;
  id_out.Signature =
      "talias(" + alias_name.ToString() + "," + aliased_type.ToString() + ")";
  return id_out;
}

GraphObserver::NodeId KytheGraphObserver::recordTypeAliasNode(
    const NameId &alias_name, const NodeId &aliased_type) {
  NodeId type_id = nodeIdForTypeAliasNode(alias_name, aliased_type);
  if (written_taliases_.insert(type_id.ToString()).second) {
    kythe::proto::VName type_vname(VNameFromNodeId(type_id));
    recorder_->BeginNode(type_vname, NodeKindID::kTAlias);
    recorder_->EndNode();
    kythe::proto::VName alias_name_vname(RecordName(alias_name));
    recorder_->AddEdge(type_vname, EdgeKindID::kNamed, alias_name_vname);
    kythe::proto::VName aliased_type_vname(VNameFromNodeId(aliased_type));
    recorder_->AddEdge(type_vname, EdgeKindID::kAliases, aliased_type_vname);
  }
  return type_id;
}

void KytheGraphObserver::recordDefinitionRange(
    const GraphObserver::Range &source_range, const NodeId &node) {
  RecordAnchor(source_range, VNameFromNodeId(node), EdgeKindID::kDefines);
}

void KytheGraphObserver::recordCompletionRange(
    const GraphObserver::Range &source_range, const NodeId &node,
    Specificity spec) {
  RecordAnchor(source_range, VNameFromNodeId(node),
               spec == Specificity::UniquelyCompletes
                   ? EdgeKindID::kUniquelyCompletes
                   : EdgeKindID::kCompletes);
}

void KytheGraphObserver::recordNamedEdge(const NodeId &node,
                                         const NameId &name) {
  recorder_->AddEdge(VNameFromNodeId(node), EdgeKindID::kNamed,
                     RecordName(name));
}

GraphObserver::NodeId KytheGraphObserver::nodeIdForNominalTypeNode(
    const NameId &name_id) {
  NodeId id_out;
  // Appending #t to a name produces the VName signature of the nominal
  // type node referring to that name. For example, the VName for a
  // forward-declared class type will look like "C#c#t".
  id_out.Signature = name_id.ToString() + "#t";
  return id_out;
}

GraphObserver::NodeId KytheGraphObserver::recordNominalTypeNode(
    const NameId &name_id) {
  NodeId id_out = nodeIdForNominalTypeNode(name_id);
  kythe::proto::VName type_vname(VNameFromNodeId(id_out));
  recorder_->BeginNode(type_vname, NodeKindID::kTNominal);
  recorder_->EndNode();
  recorder_->AddEdge(type_vname, EdgeKindID::kNamed, RecordName(name_id));
  return id_out;
}

GraphObserver::NodeId KytheGraphObserver::recordTappNode(
    const NodeId &tycon_id, const std::vector<const NodeId *> &params) {
  GraphObserver::NodeId id_out;
  // We can't just use juxtaposition here because it leads to ambiguity
  // as we can't assume that we have kind information, eg
  //   foo bar baz
  // might be
  //   foo (bar baz)
  // We'll turn it into a C-style function application:
  //   foo(bar,baz) || foo(bar(baz))
  bool comma = false;
  id_out.Signature = tycon_id.Signature;
  id_out.Signature.append("(");
  for (const auto *next_id : params) {
    if (comma) {
      id_out.Signature.append(",");
    }
    id_out.Signature.append(next_id->ToString());
    comma = true;
  }
  id_out.Signature.append(")");
  if (written_taliases_.insert(id_out.ToString()).second) {
    kythe::proto::VName tapp_vname(VNameFromNodeId(id_out));
    recorder_->BeginNode(tapp_vname, NodeKindID::kTApp);
    recorder_->EndNode();
    recorder_->AddEdge(tapp_vname, EdgeKindID::kParam,
                       VNameFromNodeId(tycon_id), 0);
    for (uint32_t param_index = 0; param_index < params.size(); ++param_index) {
      recorder_->AddEdge(tapp_vname, EdgeKindID::kParam,
                         VNameFromNodeId(*params[param_index]),
                         param_index + 1);
    }
  }
  return id_out;
}

void KytheGraphObserver::recordEnumNode(const NodeId &node_id,
                                        Completeness completeness,
                                        EnumKind enum_kind) {
  recorder_->BeginNode(VNameFromNodeId(node_id), NodeKindID::kSum);
  recorder_->AddProperty(PropertyID::kComplete,
                         CompletenessToString(completeness));
  recorder_->AddProperty(PropertyID::kSubkind,
                         enum_kind == EnumKind::Scoped ? "enumClass" : "enum");
  recorder_->EndNode();
}

void KytheGraphObserver::recordIntegerConstantNode(const NodeId &node_id,
                                                   const llvm::APSInt &Value) {
  recorder_->BeginNode(VNameFromNodeId(node_id), NodeKindID::kConstant);
  recorder_->AddProperty(PropertyID::kText, Value.toString(10));
  recorder_->EndNode();
}

void KytheGraphObserver::recordFunctionNode(const NodeId &node_id,
                                            Completeness completeness) {
  recorder_->BeginNode(VNameFromNodeId(node_id), NodeKindID::kFunction);
  recorder_->AddProperty(PropertyID::kComplete,
                         CompletenessToString(completeness));
  recorder_->EndNode();
}

void KytheGraphObserver::recordCallableNode(const NodeId &node_id) {
  recorder_->BeginNode(VNameFromNodeId(node_id), NodeKindID::kCallable);
  recorder_->EndNode();
}

void KytheGraphObserver::recordAbsNode(const NodeId &node_id) {
  recorder_->BeginNode(VNameFromNodeId(node_id), NodeKindID::kAbs);
  recorder_->EndNode();
}

void KytheGraphObserver::recordAbsVarNode(const NodeId &node_id) {
  recorder_->BeginNode(VNameFromNodeId(node_id), NodeKindID::kAbsVar);
  recorder_->EndNode();
}

void KytheGraphObserver::recordLookupNode(const NodeId &node_id,
                                          const llvm::StringRef &Name) {
  recorder_->BeginNode(VNameFromNodeId(node_id), NodeKindID::kLookup);
  recorder_->AddProperty(PropertyID::kText, Name);
  recorder_->EndNode();
}

void KytheGraphObserver::recordRecordNode(const NodeId &node_id,
                                          RecordKind kind,
                                          Completeness completeness) {
  recorder_->BeginNode(VNameFromNodeId(node_id), NodeKindID::kRecord);
  switch (kind) {
    case RecordKind::Class:
      recorder_->AddProperty(PropertyID::kSubkind, "class");
      break;
    case RecordKind::Struct:
      recorder_->AddProperty(PropertyID::kSubkind, "struct");
      break;
    case RecordKind::Union:
      recorder_->AddProperty(PropertyID::kSubkind, "union");
      break;
  };
  recorder_->AddProperty(PropertyID::kComplete,
                         CompletenessToString(completeness));
  recorder_->EndNode();
}

void KytheGraphObserver::recordTypeSpellingLocation(
    const GraphObserver::Range &type_source_range, const NodeId &type_id) {
  RecordAnchor(type_source_range, VNameFromNodeId(type_id), EdgeKindID::kRef);
}

void KytheGraphObserver::recordDeclUseLocation(
    const GraphObserver::Range &source_range, const NodeId &node) {
  RecordAnchor(source_range, VNameFromNodeId(node), EdgeKindID::kRef);
}

// TODO(zarko): Create Kythe nodes for files and file content; create and
// update internal records used to generate VNames (keep track of URIs,
// etc.)
void KytheGraphObserver::pushFile(clang::SourceLocation source_location) {
  ++file_stack_size_;
  if (source_location.isValid()) {
    if (source_location.isMacroID()) {
      source_location = SourceManager->getExpansionLoc(source_location);
    }
    assert(source_location.isFileID());
    clang::FileID file = SourceManager->getFileID(source_location);
    if (file.isInvalid()) {
      // An actually invalid location.
    } else {
      const clang::FileEntry *entry = SourceManager->getFileEntryForID(file);
      if (entry) {
        // An actual file.
        if (recorded_files_.insert(entry).second) {
          bool was_invalid = false;
          const llvm::MemoryBuffer *buf =
              SourceManager->getMemoryBufferForFile(entry, &was_invalid);
          if (was_invalid || !buf) {
            // TODO(zarko): diagnostic logging.
          } else {
            kythe::proto::VName file_vname = VNameFromFileID(file);
            recorder_->AddFileContent(file_vname, buf->getBuffer());
          }
        }
      } else {
        // A builtin location.
      }
    }
  }
}

void KytheGraphObserver::popFile() {
  if (!--file_stack_size_) {
    RecordDeferredNodes();
  }
}

}  // namespace kythe
