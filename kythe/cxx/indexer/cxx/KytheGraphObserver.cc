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
#include "kythe/cxx/common/path_utils.h"

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

static const char *FunctionSubkindToString(
    KytheGraphObserver::FunctionSubkind subkind) {
  switch (subkind) {
    case KytheGraphObserver::FunctionSubkind::None:
      return "none";
    case KytheGraphObserver::FunctionSubkind::Constructor:
      return "constructor";
    case KytheGraphObserver::FunctionSubkind::Destructor:
      return "destructor";
  }
  assert(0 && "Invalid enumerator passed to FunctionSubkindToString.");
  return "invalid-fn-subkind";
}

kythe::proto::VName KytheGraphObserver::VNameFromFileEntry(
    const clang::FileEntry *file_entry) {
  kythe::proto::VName out_name;
  if (!vfs_->get_vname(file_entry, &out_name)) {
    out_name.set_language("c++");
    llvm::StringRef working_directory = vfs_->working_directory();
    llvm::StringRef file_name(file_entry->getName());
    if (file_name.startswith(working_directory)) {
      out_name.set_path(RelativizePath(file_name, working_directory));
    } else {
      out_name.set_path(file_entry->getName());
    }
  }
  return out_name;
}

void KytheGraphObserver::AppendFileBufferSliceHashToStream(
    clang::SourceLocation loc, llvm::raw_ostream &Ostream) {
  // TODO(zarko): Does this mechanism produce sufficiently unique
  // identifiers? Ideally, we would hash the full buffer segment into
  // which `loc` points, then record `loc`'s offset.
  bool was_invalid = false;
  auto *buffer = SourceManager->getCharacterData(loc, &was_invalid);
  size_t offset = SourceManager->getFileOffset(loc);
  if (was_invalid) {
    Ostream << "!invalid[" << offset << "]";
    return;
  }
  auto loc_end = clang::Lexer::getLocForEndOfToken(
      loc, 0 /* offset from end of token */, *SourceManager, *getLangOptions());
  size_t offset_end = SourceManager->getFileOffset(loc_end);
  Ostream << HashToString(
      llvm::hash_value(llvm::StringRef(buffer, offset_end - offset)));
}

void KytheGraphObserver::AppendFullLocationToStream(
    std::vector<clang::FileID> *posted_fileids, clang::SourceLocation loc,
    llvm::raw_ostream &Ostream) {
  if (!loc.isValid()) {
    Ostream << "invalid";
    return;
  }
  if (loc.isFileID()) {
    clang::FileID file_id = SourceManager->getFileID(loc);
    const clang::FileEntry *file_entry =
        SourceManager->getFileEntryForID(file_id);
    // Don't use getPresumedLoc() since we want to ignore #line-style
    // directives.
    if (file_entry) {
      size_t offset = SourceManager->getFileOffset(loc);
      Ostream << offset;
    } else {
      AppendFileBufferSliceHashToStream(loc, Ostream);
    }
    size_t file_id_count = posted_fileids->size();
    // Don't inline the same fileid multiple times.
    // Right now we don't emit preprocessor version information, but we
    // do distinguish between FileIDs for the same FileEntry.
    for (size_t old_id = 0; old_id < file_id_count; ++old_id) {
      if (file_id == (*posted_fileids)[old_id]) {
        Ostream << "@." << old_id;
        return;
      }
    }
    posted_fileids->push_back(file_id);
    if (file_entry) {
      kythe::proto::VName file_vname(VNameFromFileEntry(file_entry));
      if (!file_vname.corpus().empty()) {
        Ostream << file_vname.corpus() << "/";
      }
      if (!file_vname.root().empty()) {
        Ostream << file_vname.root() << "/";
      }
      Ostream << file_vname.path();
    }
  } else {
    AppendFullLocationToStream(posted_fileids,
                               SourceManager->getExpansionLoc(loc), Ostream);
    Ostream << "@";
    AppendFullLocationToStream(posted_fileids,
                               SourceManager->getSpellingLoc(loc), Ostream);
  }
}

bool KytheGraphObserver::AppendRangeToStream(llvm::raw_ostream &Ostream,
                                             const Range &Range) {
  std::vector<clang::FileID> posted_fileids;
  // We want to override this here so that the names we use are filtered
  // through the vname definitions we got from the compilation unit.
  if (Range.PhysicalRange.isInvalid()) {
    return false;
  }
  AppendFullLocationToStream(&posted_fileids, Range.PhysicalRange.getBegin(),
                             Ostream);
  if (Range.PhysicalRange.getEnd() != Range.PhysicalRange.getBegin()) {
    AppendFullLocationToStream(&posted_fileids, Range.PhysicalRange.getEnd(),
                               Ostream);
  }
  if (Range.Kind == GraphObserver::Range::RangeKind::Wraith) {
    Ostream << Range.Context.ToClaimedString();
  }
  return true;
}

/// \brief Attempt to associate a `SourceLocation` with a `FileEntry` by
/// searching through the location's macro expansion history.
/// \param loc The location to associate. Any `SourceLocation` is acceptable.
/// \param source_manager The `SourceManager` that generated `loc`.
/// \return a `FileEntry` if one was found, null otherwise.
static const clang::FileEntry *SearchForFileEntry(
    clang::SourceLocation loc, clang::SourceManager *source_manager) {
  clang::FileID file_id = source_manager->getFileID(loc);
  const clang::FileEntry *out = loc.isFileID() && loc.isValid()
                                    ? source_manager->getFileEntryForID(file_id)
                                    : nullptr;
  if (out) {
    return out;
  }
  auto expansion = source_manager->getExpansionLoc(loc);
  if (expansion.isValid() && expansion != loc) {
    out = SearchForFileEntry(expansion, source_manager);
    if (out) {
      return out;
    }
  }
  auto spelling = source_manager->getSpellingLoc(loc);
  if (spelling.isValid() && spelling != loc) {
    out = SearchForFileEntry(spelling, source_manager);
  }
  return out;
}

kythe::proto::VName KytheGraphObserver::VNameFromRange(
    const GraphObserver::Range &range) {
  const clang::SourceRange &source_range = range.PhysicalRange;
  clang::SourceLocation begin = source_range.getBegin();
  clang::SourceLocation end = source_range.getEnd();
  assert(begin.isValid());
  if (!end.isValid()) {
    begin = end;
  }
  if (begin.isMacroID()) {
    begin = SourceManager->getExpansionLoc(begin);
  }
  if (end.isMacroID()) {
    end = SourceManager->getExpansionLoc(end);
  }
  kythe::proto::VName out_name;
  if (const clang::FileEntry *file_entry =
          SearchForFileEntry(begin, SourceManager)) {
    out_name.CopyFrom(VNameFromFileEntry(file_entry));
  } else if (range.Kind == GraphObserver::Range::RangeKind::Wraith) {
    VNameRefFromNodeId(range.Context).Expand(&out_name);
  } else {
    out_name.set_language("c++");
  }
  size_t begin_offset = SourceManager->getFileOffset(begin);
  size_t end_offset = SourceManager->getFileOffset(end);
  auto *const signature = out_name.mutable_signature();
  signature->append("@");
  signature->append(std::to_string(begin_offset));
  signature->append(":");
  signature->append(std::to_string(end_offset));
  if (range.Kind == GraphObserver::Range::RangeKind::Wraith) {
    signature->append("@");
    signature->append(range.Context.ToClaimedString());
  }
  out_name.set_signature(CompressString(out_name.signature()));
  return out_name;
}

void KytheGraphObserver::RecordSourceLocation(
    const VNameRef &vname, clang::SourceLocation source_location,
    PropertyID offset_id) {
  if (source_location.isMacroID()) {
    source_location = SourceManager->getExpansionLoc(source_location);
  }
  size_t offset = SourceManager->getFileOffset(source_location);
  recorder_->AddProperty(vname, offset_id, offset);
}

void KytheGraphObserver::recordMacroNode(const NodeId &macro_id) {
  recorder_->AddProperty(VNameRefFromNodeId(macro_id), NodeKindID::kMacro);
}

void KytheGraphObserver::recordExpandsRange(const Range &source_range,
                                            const NodeId &macro_id) {
  RecordAnchor(source_range, macro_id, EdgeKindID::kRefExpands,
               Claimability::Claimable);
}

void KytheGraphObserver::recordIndirectlyExpandsRange(const Range &source_range,
                                                      const NodeId &macro_id) {
  RecordAnchor(source_range, macro_id, EdgeKindID::kRefExpandsTransitive,
               Claimability::Claimable);
}

void KytheGraphObserver::recordUndefinesRange(const Range &source_range,
                                              const NodeId &macro_id) {
  RecordAnchor(source_range, macro_id, EdgeKindID::kUndefines,
               Claimability::Claimable);
}

void KytheGraphObserver::recordBoundQueryRange(const Range &source_range,
                                               const NodeId &macro_id) {
  RecordAnchor(source_range, macro_id, EdgeKindID::kRefQueries,
               Claimability::Claimable);
}

void KytheGraphObserver::recordUnboundQueryRange(const Range &source_range,
                                                 const NameId &macro_name) {
  RecordAnchor(source_range, RecordName(macro_name), EdgeKindID::kRefQueries,
               Claimability::Claimable);
}

void KytheGraphObserver::recordIncludesRange(const Range &source_range,
                                             const clang::FileEntry *File) {
  RecordAnchor(source_range, VNameFromFileEntry(File), EdgeKindID::kRefIncludes,
               Claimability::Claimable);
}

void KytheGraphObserver::recordUserDefinedNode(const NameId &name,
                                               const NodeId &node,
                                               const llvm::StringRef &kind,
                                               Completeness completeness) {
  proto::VName name_vname = RecordName(name);
  VNameRef node_vname = VNameRefFromNodeId(node);
  recorder_->AddProperty(node_vname, PropertyID::kNodeKind, kind);
  recorder_->AddProperty(node_vname, PropertyID::kComplete,
                         CompletenessToString(completeness));
  recorder_->AddEdge(node_vname, EdgeKindID::kNamed, VNameRef(name_vname));
}

void KytheGraphObserver::recordVariableNode(const NameId &name,
                                            const NodeId &node,
                                            Completeness completeness,
                                            VariableSubkind subkind) {
  proto::VName name_vname = RecordName(name);
  VNameRef node_vname = VNameRefFromNodeId(node);
  recorder_->AddProperty(node_vname, NodeKindID::kVariable);
  recorder_->AddProperty(node_vname, PropertyID::kComplete,
                         CompletenessToString(completeness));
  switch (subkind) {
    case VariableSubkind::Field:
      recorder_->AddProperty(node_vname, PropertyID::kSubkind, "field");
      break;
    case VariableSubkind::None:
      break;
  }
  recorder_->AddEdge(node_vname, EdgeKindID::kNamed, VNameRef(name_vname));
}

void KytheGraphObserver::RecordRange(const proto::VName &anchor_name,
                                     const GraphObserver::Range &range) {
  if (!deferring_nodes_ || deferred_anchors_.insert(range).second) {
    VNameRef anchor_name_ref(anchor_name);
    recorder_->AddProperty(anchor_name_ref, NodeKindID::kAnchor);
    RecordSourceLocation(anchor_name_ref, range.PhysicalRange.getBegin(),
                         PropertyID::kLocationStartOffset);
    RecordSourceLocation(anchor_name_ref, range.PhysicalRange.getEnd(),
                         PropertyID::kLocationEndOffset);
    if (const auto *file_entry = SourceManager->getFileEntryForID(
            SourceManager->getFileID(range.PhysicalRange.getBegin()))) {
      recorder_->AddEdge(anchor_name_ref, EdgeKindID::kChildOf,
                         VNameRef(VNameFromFileEntry(file_entry)));
    }
    if (range.Kind == GraphObserver::Range::RangeKind::Wraith) {
      recorder_->AddEdge(anchor_name_ref, EdgeKindID::kChildOf,
                         VNameRefFromNodeId(range.Context));
    }
  }
}

void KytheGraphObserver::MetaHookDefines(const MetadataFile &meta,
                                         const VNameRef &anchor,
                                         unsigned range_begin,
                                         unsigned range_end,
                                         const VNameRef &def) {
  auto rules = meta.rules().equal_range(range_begin);
  for (auto rule = rules.first; rule != rules.second; ++rule) {
    if (rule->second.begin == range_begin && rule->second.end == range_end &&
        rule->second.edge_in == "/kythe/edge/defines") {
      EdgeKindID edge_kind;
      if (of_spelling(rule->second.edge_out, &edge_kind)) {
        if (rule->second.reverse_edge) {
          recorder_->AddEdge(VNameRef(rule->second.vname), edge_kind, def);
        } else {
          recorder_->AddEdge(def, edge_kind, VNameRef(rule->second.vname));
        }
      } else {
        fprintf(stderr, "Unknown edge kind %s from metadata\n",
                rule->second.edge_out.c_str());
      }
    }
  }
}
proto::VName KytheGraphObserver::RecordAnchor(
    const GraphObserver::Range &source_range,
    const GraphObserver::NodeId &primary_anchored_to,
    EdgeKindID anchor_edge_kind, Claimability cl) {
  assert(!file_stack_.empty());
  proto::VName anchor_name = VNameFromRange(source_range);
  if (claimRange(source_range) || claimNode(primary_anchored_to)) {
    RecordRange(anchor_name, source_range);
    cl = Claimability::Unclaimable;
  }
  if (cl == Claimability::Unclaimable) {
    recorder_->AddEdge(VNameRef(anchor_name), anchor_edge_kind,
                       VNameRefFromNodeId(primary_anchored_to));
    if (source_range.Kind == Range::RangeKind::Physical) {
      if (anchor_edge_kind == EdgeKindID::kDefinesBinding) {
        clang::FileID def_file =
            SourceManager->getFileID(source_range.PhysicalRange.getBegin());
        const auto metas = meta_.equal_range(def_file);
        if (metas.first != metas.second) {
          auto begin = source_range.PhysicalRange.getBegin();
          if (begin.isMacroID()) {
            begin = SourceManager->getExpansionLoc(begin);
          }
          auto end = source_range.PhysicalRange.getEnd();
          if (end.isMacroID()) {
            end = SourceManager->getExpansionLoc(end);
          }
          unsigned range_begin = SourceManager->getFileOffset(begin);
          unsigned range_end = SourceManager->getFileOffset(end);
          for (auto meta = metas.first; meta != metas.second; ++meta) {
            MetaHookDefines(*meta->second, VNameRef(anchor_name), range_begin,
                            range_end, VNameRefFromNodeId(primary_anchored_to));
          }
        }
      }
    }
  }
  return anchor_name;
}

proto::VName KytheGraphObserver::RecordAnchor(
    const GraphObserver::Range &source_range,
    const kythe::proto::VName &primary_anchored_to, EdgeKindID anchor_edge_kind,
    Claimability cl) {
  assert(!file_stack_.empty());
  proto::VName anchor_name = VNameFromRange(source_range);
  if (claimRange(source_range)) {
    RecordRange(anchor_name, source_range);
    cl = Claimability::Unclaimable;
  }
  if (cl == Claimability::Unclaimable) {
    recorder_->AddEdge(VNameRef(anchor_name), anchor_edge_kind,
                       VNameRef(primary_anchored_to));
  }
  return anchor_name;
}

void KytheGraphObserver::recordCallEdge(
    const GraphObserver::Range &source_range, const NodeId &caller_id,
    const NodeId &callee_id) {
  proto::VName anchor_name = RecordAnchor(
      source_range, caller_id, EdgeKindID::kChildOf, Claimability::Claimable);
  recorder_->AddEdge(VNameRef(anchor_name), EdgeKindID::kRefCall,
                     VNameRefFromNodeId(callee_id));
}

static constexpr char const kLangCpp[] = "c++";

VNameRef KytheGraphObserver::VNameRefFromNodeId(
    const GraphObserver::NodeId &node_id) {
  VNameRef out_ref;
  out_ref.language = llvm::StringRef(kLangCpp, 3);
  if (const auto *token =
          clang::dyn_cast<KytheClaimToken>(node_id.getToken())) {
    token->DecorateVName(&out_ref);
  }
  out_ref.signature = node_id.IdentityRef();
  return out_ref;
}

kythe::proto::VName KytheGraphObserver::RecordName(
    const GraphObserver::NameId &name_id) {
  proto::VName out_vname;
  // Names don't have corpus, path or root set.
  out_vname.set_language("c++");
  const std::string name_id_string = name_id.ToString();
  out_vname.set_signature(name_id_string);
  if (!deferring_nodes_ || written_name_ids_.insert(name_id_string).second) {
    recorder_->AddProperty(VNameRef(out_vname), NodeKindID::kName);
  }
  return out_vname;
}

void KytheGraphObserver::recordParamEdge(const NodeId &param_of_id,
                                         uint32_t ordinal,
                                         const NodeId &param_id) {
  recorder_->AddEdge(VNameRefFromNodeId(param_of_id), EdgeKindID::kParam,
                     VNameRefFromNodeId(param_id), ordinal);
}

void KytheGraphObserver::recordChildOfEdge(const NodeId &child_id,
                                           const NodeId &parent_id) {
  recorder_->AddEdge(VNameRefFromNodeId(child_id), EdgeKindID::kChildOf,
                     VNameRefFromNodeId(parent_id));
}

void KytheGraphObserver::recordTypeEdge(const NodeId &term_id,
                                        const NodeId &type_id) {
  recorder_->AddEdge(VNameRefFromNodeId(term_id), EdgeKindID::kHasType,
                     VNameRefFromNodeId(type_id));
}

void KytheGraphObserver::recordCallableAsEdge(const NodeId &from_id,
                                              const NodeId &to_id) {
  recorder_->AddEdge(VNameRefFromNodeId(from_id), EdgeKindID::kCallableAs,
                     VNameRefFromNodeId(to_id));
}

void KytheGraphObserver::recordSpecEdge(const NodeId &term_id,
                                        const NodeId &type_id,
                                        Confidence conf) {
  switch (conf) {
    case Confidence::NonSpeculative:
      recorder_->AddEdge(VNameRefFromNodeId(term_id), EdgeKindID::kSpecializes,
                         VNameRefFromNodeId(type_id));
      break;
    case Confidence::Speculative:
      recorder_->AddEdge(VNameRefFromNodeId(term_id),
                         EdgeKindID::kSpecializesSpeculative,
                         VNameRefFromNodeId(type_id));
      break;
  }
}

void KytheGraphObserver::recordInstEdge(const NodeId &term_id,
                                        const NodeId &type_id,
                                        Confidence conf) {
  switch (conf) {
    case Confidence::NonSpeculative:
      recorder_->AddEdge(VNameRefFromNodeId(term_id), EdgeKindID::kInstantiates,
                         VNameRefFromNodeId(type_id));
      break;
    case Confidence::Speculative:
      recorder_->AddEdge(VNameRefFromNodeId(term_id),
                         EdgeKindID::kInstantiatesSpeculative,
                         VNameRefFromNodeId(type_id));
      break;
  }
}

GraphObserver::NodeId KytheGraphObserver::nodeIdForTypeAliasNode(
    const NameId &alias_name, const NodeId &aliased_type) {
  return NodeId(&type_token_, "talias(" + alias_name.ToString() + "," +
                                  aliased_type.ToClaimedString() + ")");
}

GraphObserver::NodeId KytheGraphObserver::recordTypeAliasNode(
    const NameId &alias_name, const NodeId &aliased_type) {
  NodeId type_id = nodeIdForTypeAliasNode(alias_name, aliased_type);
  if (!deferring_nodes_ ||
      written_types_.insert(type_id.ToClaimedString()).second) {
    VNameRef type_vname(VNameRefFromNodeId(type_id));
    recorder_->AddProperty(type_vname, NodeKindID::kTAlias);
    kythe::proto::VName alias_name_vname(RecordName(alias_name));
    recorder_->AddEdge(type_vname, EdgeKindID::kNamed,
                       VNameRef(alias_name_vname));
    VNameRef aliased_type_vname(VNameRefFromNodeId(aliased_type));
    recorder_->AddEdge(type_vname, EdgeKindID::kAliases,
                       VNameRef(aliased_type_vname));
  }
  return type_id;
}

void KytheGraphObserver::recordDocumentationRange(
    const GraphObserver::Range &source_range, const NodeId &node) {
  RecordAnchor(source_range, node, EdgeKindID::kDocuments,
               Claimability::Claimable);
}

void KytheGraphObserver::recordFullDefinitionRange(
    const GraphObserver::Range &source_range, const NodeId &node) {
  RecordAnchor(source_range, node, EdgeKindID::kDefinesFull,
               Claimability::Claimable);
}

void KytheGraphObserver::recordDefinitionBindingRange(
    const GraphObserver::Range &binding_range, const NodeId &node) {
  RecordAnchor(binding_range, node, EdgeKindID::kDefinesBinding,
               Claimability::Claimable);
}

void KytheGraphObserver::recordDefinitionRangeWithBinding(
    const GraphObserver::Range &source_range,
    const GraphObserver::Range &binding_range, const NodeId &node) {
  RecordAnchor(source_range, node, EdgeKindID::kDefinesFull,
               Claimability::Claimable);
  RecordAnchor(binding_range, node, EdgeKindID::kDefinesBinding,
               Claimability::Claimable);
}

void KytheGraphObserver::recordCompletionRange(
    const GraphObserver::Range &source_range, const NodeId &node,
    Specificity spec) {
  RecordAnchor(source_range, node, spec == Specificity::UniquelyCompletes
                                       ? EdgeKindID::kUniquelyCompletes
                                       : EdgeKindID::kCompletes,
               Claimability::Unclaimable);
}

void KytheGraphObserver::recordNamedEdge(const NodeId &node,
                                         const NameId &name) {
  recorder_->AddEdge(VNameRefFromNodeId(node), EdgeKindID::kNamed,
                     VNameRef(RecordName(name)));
}

GraphObserver::NodeId KytheGraphObserver::nodeIdForNominalTypeNode(
    const NameId &name_id) {
  // Appending #t to a name produces the VName signature of the nominal
  // type node referring to that name. For example, the VName for a
  // forward-declared class type will look like "C#c#t".
  return NodeId(&type_token_, name_id.ToString() + "#t");
}

GraphObserver::NodeId KytheGraphObserver::recordNominalTypeNode(
    const NameId &name_id) {
  NodeId id_out = nodeIdForNominalTypeNode(name_id);
  if (!deferring_nodes_ ||
      written_types_.insert(id_out.ToClaimedString()).second) {
    VNameRef type_vname(VNameRefFromNodeId(id_out));
    recorder_->AddProperty(type_vname, NodeKindID::kTNominal);
    recorder_->AddEdge(type_vname, EdgeKindID::kNamed,
                       VNameRef(RecordName(name_id)));
  }
  return id_out;
}

GraphObserver::NodeId KytheGraphObserver::recordTappNode(
    const NodeId &tycon_id, const std::vector<const NodeId *> &params) {
  // We can't just use juxtaposition here because it leads to ambiguity
  // as we can't assume that we have kind information, eg
  //   foo bar baz
  // might be
  //   foo (bar baz)
  // We'll turn it into a C-style function application:
  //   foo(bar,baz) || foo(bar(baz))
  std::string identity;
  llvm::raw_string_ostream ostream(identity);
  bool comma = false;
  ostream << tycon_id.ToClaimedString();
  ostream << "(";
  for (const auto *next_id : params) {
    if (comma) {
      ostream << ",";
    }
    ostream << next_id->ToClaimedString();
    comma = true;
  }
  ostream << ")";
  GraphObserver::NodeId id_out(&type_token_, ostream.str());
  if (!deferring_nodes_ ||
      written_types_.insert(id_out.ToClaimedString()).second) {
    VNameRef tapp_vname(VNameRefFromNodeId(id_out));
    recorder_->AddProperty(tapp_vname, NodeKindID::kTApp);
    recorder_->AddEdge(tapp_vname, EdgeKindID::kParam,
                       VNameRefFromNodeId(tycon_id), 0);
    for (uint32_t param_index = 0; param_index < params.size(); ++param_index) {
      recorder_->AddEdge(tapp_vname, EdgeKindID::kParam,
                         VNameRefFromNodeId(*params[param_index]),
                         param_index + 1);
    }
  }
  return id_out;
}

void KytheGraphObserver::recordEnumNode(const NodeId &node_id,
                                        Completeness completeness,
                                        EnumKind enum_kind) {
  VNameRef node_vname = VNameRefFromNodeId(node_id);
  recorder_->AddProperty(node_vname, NodeKindID::kSum);
  recorder_->AddProperty(node_vname, PropertyID::kComplete,
                         CompletenessToString(completeness));
  recorder_->AddProperty(node_vname, PropertyID::kSubkind,
                         enum_kind == EnumKind::Scoped ? "enumClass" : "enum");
}

void KytheGraphObserver::recordIntegerConstantNode(const NodeId &node_id,
                                                   const llvm::APSInt &Value) {
  VNameRef node_vname(VNameRefFromNodeId(node_id));
  recorder_->AddProperty(node_vname, NodeKindID::kConstant);
  recorder_->AddProperty(node_vname, PropertyID::kText, Value.toString(10));
}

void KytheGraphObserver::recordFunctionNode(const NodeId &node_id,
                                            Completeness completeness,
                                            FunctionSubkind subkind) {
  VNameRef node_vname = VNameRefFromNodeId(node_id);
  recorder_->AddProperty(node_vname, NodeKindID::kFunction);
  recorder_->AddProperty(node_vname, PropertyID::kComplete,
                         CompletenessToString(completeness));
  if (subkind != FunctionSubkind::None) {
    recorder_->AddProperty(node_vname, PropertyID::kSubkind,
                           FunctionSubkindToString(subkind));
  }
}

void KytheGraphObserver::recordCallableNode(const NodeId &node_id) {
  recorder_->AddProperty(VNameRefFromNodeId(node_id), NodeKindID::kCallable);
}

void KytheGraphObserver::recordAbsNode(const NodeId &node_id) {
  recorder_->AddProperty(VNameRefFromNodeId(node_id), NodeKindID::kAbs);
}

void KytheGraphObserver::recordAbsVarNode(const NodeId &node_id) {
  recorder_->AddProperty(VNameRefFromNodeId(node_id), NodeKindID::kAbsVar);
}

void KytheGraphObserver::recordLookupNode(const NodeId &node_id,
                                          const llvm::StringRef &Name) {
  VNameRef node_vname = VNameRefFromNodeId(node_id);
  recorder_->AddProperty(node_vname, NodeKindID::kLookup);
  recorder_->AddProperty(node_vname, PropertyID::kText, Name);
}

void KytheGraphObserver::recordRecordNode(const NodeId &node_id,
                                          RecordKind kind,
                                          Completeness completeness) {
  VNameRef node_vname = VNameRefFromNodeId(node_id);
  recorder_->AddProperty(node_vname, NodeKindID::kRecord);
  switch (kind) {
    case RecordKind::Class:
      recorder_->AddProperty(node_vname, PropertyID::kSubkind, "class");
      break;
    case RecordKind::Struct:
      recorder_->AddProperty(node_vname, PropertyID::kSubkind, "struct");
      break;
    case RecordKind::Union:
      recorder_->AddProperty(node_vname, PropertyID::kSubkind, "union");
      break;
  };
  recorder_->AddProperty(node_vname, PropertyID::kComplete,
                         CompletenessToString(completeness));
}

void KytheGraphObserver::recordTypeSpellingLocation(
    const GraphObserver::Range &type_source_range, const NodeId &type_id,
    Claimability claimability) {
  RecordAnchor(type_source_range, type_id, EdgeKindID::kRef, claimability);
}

void KytheGraphObserver::recordExtendsEdge(const NodeId &from, const NodeId &to,
                                           bool is_virtual,
                                           clang::AccessSpecifier specifier) {
  switch (specifier) {
    case clang::AccessSpecifier::AS_public:
      recorder_->AddEdge(VNameRefFromNodeId(from),
                         is_virtual ? EdgeKindID::kExtendsPublicVirtual
                                    : EdgeKindID::kExtendsPublic,
                         VNameRefFromNodeId(to));
      break;
    case clang::AccessSpecifier::AS_protected:
      recorder_->AddEdge(VNameRefFromNodeId(from),
                         is_virtual ? EdgeKindID::kExtendsProtectedVirtual
                                    : EdgeKindID::kExtendsProtected,
                         VNameRefFromNodeId(to));
      break;
    case clang::AccessSpecifier::AS_private:
      recorder_->AddEdge(VNameRefFromNodeId(from),
                         is_virtual ? EdgeKindID::kExtendsPrivateVirtual
                                    : EdgeKindID::kExtendsPrivate,
                         VNameRefFromNodeId(to));
      break;
    default:
      recorder_->AddEdge(
          VNameRefFromNodeId(from),
          is_virtual ? EdgeKindID::kExtendsVirtual : EdgeKindID::kExtends,
          VNameRefFromNodeId(to));
  }
}

void KytheGraphObserver::recordDeclUseLocationInDocumentation(
    const GraphObserver::Range &source_range, const NodeId &node) {
  RecordAnchor(source_range, node, EdgeKindID::kRefDoc,
               Claimability::Claimable);
}

void KytheGraphObserver::recordDeclUseLocation(
    const GraphObserver::Range &source_range, const NodeId &node,
    Claimability claimability) {
  RecordAnchor(source_range, node, EdgeKindID::kRef, claimability);
}

void KytheGraphObserver::applyMetadataFile(clang::FileID id,
                                           const clang::FileEntry *file) {
  const llvm::MemoryBuffer *buffer =
      SourceManager->getMemoryBufferForFile(file);
  if (!buffer) {
    fprintf(stderr, "Couldn't get content for %s\n", file->getName());
    return;
  }
  std::string error;
  auto metadata = MetadataFile::LoadFromJSON(buffer->getBuffer(), &error);
  if (metadata) {
    meta_.emplace(id, std::move(metadata));
  } else {
    fprintf(stderr, "Couldn't load %s: %s\n", file->getName(), error.c_str());
  }
}

void KytheGraphObserver::pushFile(clang::SourceLocation blame_location,
                                  clang::SourceLocation source_location) {
  PreprocessorContext previous_context =
      file_stack_.empty() ? starting_context_ : file_stack_.back().context;
  bool has_previous_uid = !file_stack_.empty();
  llvm::sys::fs::UniqueID previous_uid;
  if (has_previous_uid) {
    previous_uid = file_stack_.back().uid;
  }
  file_stack_.push_back(FileState{});
  FileState &state = file_stack_.back();
  state.claimed = true;
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
        state.vname = state.base_vname = VNameFromFileEntry(entry);
        state.uid = entry->getUniqueID();
        // Attempt to compute the state-amended VName using the state table.
        // If we aren't working under any context, we won't end up making the
        // VName more specific.
        if (file_stack_.size() == 1) {
          // Start state.
          state.context = starting_context_;
        } else if (has_previous_uid && !previous_context.empty() &&
                   blame_location.isValid() && blame_location.isFileID()) {
          unsigned offset = SourceManager->getFileOffset(blame_location);
          const auto path_info = path_to_context_data_.find(previous_uid);
          if (path_info != path_to_context_data_.end()) {
            const auto context_info = path_info->second.find(previous_context);
            if (context_info != path_info->second.end()) {
              const auto offset_info = context_info->second.find(offset);
              if (offset_info != context_info->second.end()) {
                state.context = offset_info->second;
              } else {
                fprintf(stderr,
                        "Warning: when looking for %s[%s]:%u: missing source "
                        "offset\n",
                        vfs_->get_debug_uid_string(previous_uid).c_str(),
                        previous_context.c_str(), offset);
              }
            } else {
              fprintf(stderr,
                      "Warning: when looking for %s[%s]:%u: missing source "
                      "context\n",
                      vfs_->get_debug_uid_string(previous_uid).c_str(),
                      previous_context.c_str(), offset);
            }
          } else {
            fprintf(
                stderr,
                "Warning: when looking for %s[%s]:%u: missing source path\n",
                vfs_->get_debug_uid_string(previous_uid).c_str(),
                previous_context.c_str(), offset);
          }
        }
        state.vname.set_signature(state.context + state.vname.signature());
        if (client_->Claim(claimant_, state.vname)) {
          if (recorded_files_.insert(entry).second) {
            bool was_invalid = false;
            const llvm::MemoryBuffer *buf =
                SourceManager->getMemoryBufferForFile(entry, &was_invalid);
            if (was_invalid || !buf) {
              // TODO(zarko): diagnostic logging.
            } else {
              recorder_->AddFileContent(VNameRef(state.base_vname),
                                        buf->getBuffer());
            }
          }
        } else {
          state.claimed = false;
        }
        KytheClaimToken token;
        token.set_vname(state.vname);
        token.set_rough_claimed(state.claimed);
        claim_checked_files_.emplace(file, token);
      } else {
        // A builtin location.
      }
    }
  }
}

void KytheGraphObserver::popFile() {
  assert(!file_stack_.empty());
  FileState state = file_stack_.back();
  file_stack_.pop_back();
  if (file_stack_.empty()) {
    deferred_anchors_.clear();
  }
}

bool KytheGraphObserver::claimRange(const GraphObserver::Range &range) {
  return (range.Kind == GraphObserver::Range::RangeKind::Wraith &&
          claimNode(range.Context)) ||
         claimLocation(range.PhysicalRange.getBegin());
}

bool KytheGraphObserver::claimLocation(clang::SourceLocation source_location) {
  if (!source_location.isValid()) {
    return true;
  }
  if (source_location.isMacroID()) {
    source_location = SourceManager->getExpansionLoc(source_location);
  }
  assert(source_location.isFileID());
  clang::FileID file = SourceManager->getFileID(source_location);
  if (file.isInvalid()) {
    return true;
  }
  auto token = claim_checked_files_.find(file);
  return token != claim_checked_files_.end() ? token->second.rough_claimed()
                                             : false;
}

void KytheGraphObserver::AddContextInformation(
    const std::string &path, const PreprocessorContext &context,
    unsigned offset, const PreprocessorContext &dest_context) {
  auto found_file = vfs_->status(path);
  if (found_file) {
    path_to_context_data_[found_file->getUniqueID()][context][offset] =
        dest_context;
  } else {
    fprintf(stderr, "WARNING: Path %s could not be mapped to a VFS record.\n",
            path.c_str());
  }
}

const GraphObserver::ClaimToken *KytheGraphObserver::getClaimTokenForLocation(
    clang::SourceLocation source_location) {
  if (!source_location.isValid()) {
    return getDefaultClaimToken();
  }
  if (source_location.isMacroID()) {
    source_location = SourceManager->getExpansionLoc(source_location);
  }
  assert(source_location.isFileID());
  clang::FileID file = SourceManager->getFileID(source_location);
  if (file.isInvalid()) {
    return getDefaultClaimToken();
  }
  auto token = claim_checked_files_.find(file);
  return token != claim_checked_files_.end() ? &token->second
                                             : getDefaultClaimToken();
}

const GraphObserver::ClaimToken *KytheGraphObserver::getClaimTokenForRange(
    const clang::SourceRange &range) {
  return getClaimTokenForLocation(range.getBegin());
}

void *KytheClaimToken::clazz_ = nullptr;

}  // namespace kythe
