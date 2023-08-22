/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

#include <optional>
#include <string>

#include "IndexerASTHooks.h"
#include "absl/flags/flag.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
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
#include "clang/Basic/Specifiers.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/cxx/common/schema/edges.h"
#include "kythe/cxx/extractor/language.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/SHA1.h"

ABSL_FLAG(bool, fail_on_unimplemented_builtin, false,
          "Fail indexer if we encounter a builtin we do not handle");

namespace kythe {
namespace {

/// \brief Clang builtins whose name and token match.
constexpr absl::string_view kUniformClangBuiltins[] = {
    "void",
    "bool",
    "_Bool",
    "signed char",
    "char",
    "char16_t",
    "char32_t",
    "wchar_t",
    "short",
    "int",
    "long",
    "long long",
    "unsigned char",
    "unsigned short",
    "unsigned int",
    "unsigned long",
    "unsigned long long",
    "float",
    "double",
    "long double",
    "auto",
    "__int128",
    "unsigned __int128",
    "SEL",
    "id",
    "TypeUnion",
    "__float128",
    // Additional aarch64 builtins.
    "__SVInt8_t",
    "__SVInt16_t",
    "__SVInt32_t",
    "__SVInt64_t",
    "__SVUint8_t",
    "__SVUint16_t",
    "__SVUint32_t",
    "__SVUint64_t",
    "__SVFloat16_t",
    "__SVFloat32_t",
    "__SVFloat64_t",
    "__SVBFloat16_t",
    "__clang_svint8x2_t",
    "__clang_svint16x2_t",
    "__clang_svint32x2_t",
    "__clang_svint64x2_t",
    "__clang_svuint8x2_t",
    "__clang_svuint16x2_t",
    "__clang_svuint32x2_t",
    "__clang_svuint64x2_t",
    "__clang_svfloat16x2_t",
    "__clang_svfloat32x2_t",
    "__clang_svfloat64x2_t",
    "__clang_svbfloat16x2_t",
    "__clang_svint8x3_t",
    "__clang_svint16x3_t",
    "__clang_svint32x3_t",
    "__clang_svint64x3_t",
    "__clang_svuint8x3_t",
    "__clang_svuint16x3_t",
    "__clang_svuint32x3_t",
    "__clang_svuint64x3_t",
    "__clang_svfloat16x3_t",
    "__clang_svfloat32x3_t",
    "__clang_svfloat64x3_t",
    "__clang_svbfloat16x3_t",
    "__clang_svint8x4_t",
    "__clang_svint16x4_t",
    "__clang_svint32x4_t",
    "__clang_svint64x4_t",
    "__clang_svuint8x4_t",
    "__clang_svuint16x4_t",
    "__clang_svuint32x4_t",
    "__clang_svuint64x4_t",
    "__clang_svfloat16x4_t",
    "__clang_svfloat32x4_t",
    "__clang_svfloat64x4_t",
    "__clang_svbfloat16x4_t",
    "__SVBool_t",
    "__clang_svboolx2_t",
    "__clang_svboolx4_t",
    "__SVCount_t",
};

struct ClaimedStringFormatter {
  void operator()(std::string* out, const GraphObserver::NodeId& id) {
    out->append(id.ToClaimedString());
  }
};

absl::string_view ConvertRef(llvm::StringRef ref) {
  return absl::string_view(ref.data(), ref.size());
}

const char* VisibilityPropertyValue(clang::AccessSpecifier access) {
  switch (access) {
    case clang::AS_public:
      return "public";
    case clang::AS_protected:
      return "protected";
    case clang::AS_private:
      return "private";
    case clang::AS_none:
      return "";
  }
  LOG(FATAL) << "Unknown access specifier passed to VisibilityPropertyValue< "
             << access;
  return "";
}

}  // anonymous namespace

using clang::SourceLocation;
using llvm::StringRef;

static const char* CompletenessToString(
    KytheGraphObserver::Completeness completeness) {
  switch (completeness) {
    case KytheGraphObserver::Completeness::Definition:
      return "definition";
    case KytheGraphObserver::Completeness::Complete:
      return "complete";
    case KytheGraphObserver::Completeness::Incomplete:
      return "incomplete";
  }
  LOG(FATAL) << "Invalid enumerator passed to CompletenessToString.";
  return "invalid-completeness";
}

static const char* FunctionSubkindToString(
    KytheGraphObserver::FunctionSubkind subkind) {
  switch (subkind) {
    case KytheGraphObserver::FunctionSubkind::None:
      return "none";
    case KytheGraphObserver::FunctionSubkind::Constructor:
      return "constructor";
    case KytheGraphObserver::FunctionSubkind::Destructor:
      return "destructor";
    case KytheGraphObserver::FunctionSubkind::Initializer:
      return "initializer";
  }
  LOG(FATAL) << "Invalid enumerator passed to FunctionSubkindToString.";
  return "invalid-fn-subkind";
}

kythe::proto::VName KytheGraphObserver::VNameFromFileEntry(
    const clang::FileEntry* file_entry) const {
  kythe::proto::VName out_name;
  if (!vfs_->get_vname(file_entry, &out_name)) {
    llvm::StringRef working_directory = vfs_->working_directory();
    llvm::StringRef file_name(file_entry->getName());
    if (file_name.startswith(working_directory)) {
      out_name.set_path(
          RelativizePath(ConvertRef(file_name), ConvertRef(working_directory)));
    } else {
      out_name.set_path(std::string(file_entry->getName()));
    }
    out_name.set_corpus(claimant_.corpus());
  }
  return out_name;
}

void KytheGraphObserver::AppendFileBufferSliceHashToStream(
    clang::SourceLocation loc, llvm::raw_ostream& Ostream) const {
  // TODO(zarko): Does this mechanism produce sufficiently unique
  // identifiers? Ideally, we would hash the full buffer segment into
  // which `loc` points, then record `loc`'s offset.
  bool was_invalid = false;
  auto* buffer = SourceManager->getCharacterData(loc, &was_invalid);
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
    std::vector<clang::FileID>* posted_fileids, clang::SourceLocation loc,
    llvm::raw_ostream& Ostream) const {
  if (!loc.isValid()) {
    Ostream << "invalid";
    return;
  }
  if (loc.isFileID()) {
    clang::FileID file_id = SourceManager->getFileID(loc);
    const clang::FileEntry* file_entry =
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

void KytheGraphObserver::AppendRangeToStream(llvm::raw_ostream& Ostream,
                                             const Range& Range) const {
  std::vector<clang::FileID> posted_fileids;
  // We want to override this here so that the names we use are filtered
  // through the vname definitions we got from the compilation unit.
  AppendFullLocationToStream(&posted_fileids, Range.PhysicalRange.getBegin(),
                             Ostream);
  if (Range.PhysicalRange.getEnd() != Range.PhysicalRange.getBegin()) {
    AppendFullLocationToStream(&posted_fileids, Range.PhysicalRange.getEnd(),
                               Ostream);
  }
  if (Range.Kind == GraphObserver::Range::RangeKind::Wraith) {
    Ostream << Range.Context.ToClaimedString();
  }
}

/// \brief Attempt to associate a `SourceLocation` with a `FileEntry` by
/// searching through the location's macro expansion history.
/// \param loc The location to associate. Any `SourceLocation` is acceptable.
/// \param source_manager The `SourceManager` that generated `loc`.
/// \return a `FileEntry` if one was found, null otherwise.
static const clang::FileEntry* SearchForFileEntry(
    clang::SourceLocation loc, clang::SourceManager* source_manager) {
  clang::FileID file_id = source_manager->getFileID(loc);
  const clang::FileEntry* out = loc.isFileID() && loc.isValid()
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

kythe::proto::VName KytheGraphObserver::StampedVNameFromRange(
    const GraphObserver::Range& range, const GraphObserver::NodeId& stamp) {
  auto vname = VNameFromRange(range);
  vname.mutable_signature()->append("@");
  vname.mutable_signature()->append(stamp.ToClaimedString());
  return vname;
}

kythe::proto::VName KytheGraphObserver::VNameFromRange(
    const GraphObserver::Range& range) {
  kythe::proto::VName out_name;
  if (range.Kind == GraphObserver::Range::RangeKind::Implicit &&
      !range.PhysicalRange.getBegin().isValid()) {
    VNameRefFromNodeId(range.Context).Expand(&out_name);
    out_name.mutable_signature()->append("@syntactic");
  } else {
    const clang::SourceRange& source_range = range.PhysicalRange;
    clang::SourceLocation begin = source_range.getBegin();
    clang::SourceLocation end = source_range.getEnd();
    CHECK(begin.isValid());
    if (!end.isValid()) {
      end = begin;
    }
    if (begin.isMacroID()) {
      begin = SourceManager->getExpansionLoc(begin);
    }
    if (end.isMacroID()) {
      end = SourceManager->getExpansionLoc(end);
    }
    if (const clang::FileEntry* file_entry =
            SearchForFileEntry(begin, SourceManager)) {
      out_name.CopyFrom(VNameFromFileEntry(file_entry));
    } else if (range.Kind == GraphObserver::Range::RangeKind::Wraith) {
      VNameRefFromNodeId(range.Context).Expand(&out_name);
    } else {
      out_name.set_corpus(default_token_.vname().corpus());
    }
    size_t begin_offset = SourceManager->getFileOffset(begin);
    size_t end_offset = SourceManager->getFileOffset(end);
    auto* const signature = out_name.mutable_signature();
    signature->append("@");
    signature->append(std::to_string(begin_offset));
    signature->append(":");
    signature->append(std::to_string(end_offset));
    if (range.Kind == GraphObserver::Range::RangeKind::Wraith ||
        range.Kind == GraphObserver::Range::RangeKind::Implicit) {
      signature->append("@");
      signature->append(range.Context.ToClaimedString());
    }
  }
  out_name.set_language(supported_language::kIndexerLang);
  if (!build_config_.empty()) {
    absl::StrAppend(out_name.mutable_signature(), "%", build_config_);
  }
  out_name.set_signature(CompressAnchorSignature(out_name.signature()));
  return out_name;
}

void KytheGraphObserver::RecordSourceLocation(
    const VNameRef& vname, clang::SourceLocation source_location,
    PropertyID offset_id) {
  if (source_location.isMacroID()) {
    source_location = SourceManager->getExpansionLoc(source_location);
  }
  size_t offset = SourceManager->getFileOffset(source_location);
  recorder_->AddProperty(vname, offset_id, offset);
}

void KytheGraphObserver::recordMacroNode(const NodeId& macro_id) {
  recorder_->AddProperty(VNameRefFromNodeId(macro_id), NodeKindID::kMacro);
}

void KytheGraphObserver::recordExpandsRange(const Range& source_range,
                                            const NodeId& macro_id) {
  RecordAnchor(source_range, macro_id, EdgeKindID::kRefExpands,
               Claimability::Claimable);
}

void KytheGraphObserver::recordIndirectlyExpandsRange(const Range& source_range,
                                                      const NodeId& macro_id) {
  RecordAnchor(source_range, macro_id, EdgeKindID::kRefExpandsTransitive,
               Claimability::Claimable);
}

void KytheGraphObserver::recordUndefinesRange(const Range& source_range,
                                              const NodeId& macro_id) {
  RecordAnchor(source_range, macro_id, EdgeKindID::kUndefines,
               Claimability::Claimable);
}

void KytheGraphObserver::recordBoundQueryRange(const Range& source_range,
                                               const NodeId& macro_id) {
  RecordAnchor(source_range, macro_id, EdgeKindID::kRefQueries,
               Claimability::Claimable);
}

void KytheGraphObserver::recordIncludesRange(const Range& source_range,
                                             const clang::FileEntry* File) {
  RecordAnchor(source_range, VNameFromFileEntry(File), EdgeKindID::kRefIncludes,
               Claimability::Claimable);
}

void KytheGraphObserver::recordUserDefinedNode(
    const NodeId& node, llvm::StringRef kind,
    const std::optional<Completeness> completeness) {
  VNameRef node_vname = VNameRefFromNodeId(node);
  recorder_->AddProperty(node_vname, PropertyID::kNodeKind, ConvertRef(kind));
  if (completeness) {
    recorder_->AddProperty(node_vname, PropertyID::kComplete,
                           CompletenessToString(*completeness));
  }
}

void KytheGraphObserver::recordVariableNode(
    const NodeId& node, Completeness completeness, VariableSubkind subkind,
    const std::optional<MarkedSource>& marked_source) {
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
  AddMarkedSource(node_vname, marked_source);
}

void KytheGraphObserver::recordNamespaceNode(
    const NodeId& node, const std::optional<MarkedSource>& marked_source) {
  VNameRef node_vname = VNameRefFromNodeId(node);
  if (written_namespaces_.insert(node.ToClaimedString()).second) {
    recorder_->AddProperty(node_vname, NodeKindID::kPackage);
    recorder_->AddProperty(node_vname, PropertyID::kSubkind, "namespace");
    AddMarkedSource(node_vname, marked_source);
  }
}

void KytheGraphObserver::RecordRange(const proto::VName& anchor_name,
                                     const GraphObserver::Range& range) {
  if (!deferring_nodes_ || deferred_anchors_.insert(range).second) {
    UnconditionalRecordRange(anchor_name, range);
  }
}

void KytheGraphObserver::UnconditionalRecordRange(
    const proto::VName& anchor_name, const GraphObserver::Range& range) {
  VNameRef anchor_name_ref(anchor_name);
  recorder_->AddProperty(anchor_name_ref, NodeKindID::kAnchor);
  if (range.Kind == GraphObserver::Range::RangeKind::Implicit) {
    recorder_->AddProperty(anchor_name_ref, PropertyID::kSubkind, "implicit");
  }
  if (range.PhysicalRange.getBegin().isValid()) {
    RecordSourceLocation(anchor_name_ref, range.PhysicalRange.getBegin(),
                         PropertyID::kLocationStartOffset);
    RecordSourceLocation(anchor_name_ref, range.PhysicalRange.getEnd(),
                         PropertyID::kLocationEndOffset);
  }
  if (range.Kind == GraphObserver::Range::RangeKind::Wraith) {
    recorder_->AddEdge(anchor_name_ref, EdgeKindID::kChildOfContext,
                       VNameRefFromNodeId(range.Context));
  }
  if (!build_config_.empty()) {
    recorder_->AddProperty(anchor_name_ref, PropertyID::kBuildConfig,
                           build_config_);
  }
}

void KytheGraphObserver::MetaHookDefines(const MetadataFile& meta,
                                         const VNameRef& anchor,
                                         unsigned range_begin,
                                         unsigned range_end,
                                         const VNameRef& decl) {
  auto rules = meta.rules().equal_range(range_begin);
  for (auto rule = rules.first; rule != rules.second; ++rule) {
    if (rule->second.begin == range_begin && rule->second.end == range_end &&
        (rule->second.edge_in == kythe::common::schema::kDefines ||
         rule->second.edge_in == kythe::common::schema::kDefinesBinding)) {
      VNameRef remote(rule->second.vname);
      EdgeKindID edge_kind;
      std::string new_signature;
      if (rule->second.generate_anchor) {
        // Distinguish these anchors from ordinary ones for easier debugging.
        new_signature = "@@m";
        new_signature.append(std::to_string(rule->second.anchor_begin));
        new_signature.append("-");
        new_signature.append(std::to_string(rule->second.anchor_end));
        remote.set_signature(new_signature);
        recorder_->AddProperty(remote, NodeKindID::kAnchor);
        recorder_->AddProperty(remote, PropertyID::kLocationStartOffset,
                               rule->second.anchor_begin);
        recorder_->AddProperty(remote, PropertyID::kLocationEndOffset,
                               rule->second.anchor_end);
      }
      if (of_spelling(rule->second.edge_out, &edge_kind)) {
        if (rule->second.reverse_edge) {
          recorder_->AddEdge(remote, edge_kind, decl);
        } else {
          recorder_->AddEdge(decl, edge_kind, remote);
        }
      } else {
        absl::FPrintF(stderr, "Unknown edge kind %s from metadata\n",
                      rule->second.edge_out);
      }
    }
  }

  // Emit file-scope edges, if the anchor VName directly corresponds to a file.
  if (!anchor.path().empty()) {
    VNameRef file_vname(anchor);
    file_vname.set_signature("");
    file_vname.set_language("");
    for (const auto& rule : meta.file_scope_rules()) {
      EdgeKindID edge_kind;
      if (of_spelling(rule.edge_out, &edge_kind)) {
        if (MarkFileMetaEdgeEmitted(file_vname, meta)) {
          VNameRef remote(rule.vname);
          if (rule.reverse_edge) {
            recorder_->AddEdge(remote, edge_kind, file_vname);
          } else {
            recorder_->AddEdge(file_vname, edge_kind, remote);
          }
        }
      } else {
        absl::FPrintF(stderr, "Unknown edge kind %s from metadata\n",
                      rule.edge_out);
      }
    }
  }
}

bool KytheGraphObserver::MarkFileMetaEdgeEmitted(const VNameRef& file_decl,
                                                 const MetadataFile& meta) {
  return file_meta_edges_emitted_
      .emplace(std::string(file_decl.path()), std::string(file_decl.corpus()),
               std::string(file_decl.root()), std::string(meta.id()))
      .second;
}

void KytheGraphObserver::ApplyMetadataRules(
    const GraphObserver::Range& source_range,
    const GraphObserver::NodeId& primary_anchored_to_decl,
    const std::optional<GraphObserver::NodeId>& primary_anchored_to_def,
    EdgeKindID anchor_edge_kind, const kythe::proto::VName& anchor_name) {
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
                          range_end,
                          VNameRefFromNodeId(primary_anchored_to_decl));
          if (primary_anchored_to_def) {
            MetaHookDefines(*meta->second, VNameRef(anchor_name), range_begin,
                            range_end,
                            VNameRefFromNodeId(*primary_anchored_to_def));
          }
        }
      }
    }
  }
}

void KytheGraphObserver::RecordStampedAnchor(
    const GraphObserver::Range& source_range,
    const GraphObserver::NodeId& primary_anchored_to_decl,
    const std::optional<GraphObserver::NodeId>& primary_anchored_to_def,
    EdgeKindID anchor_edge_kind, const GraphObserver::NodeId& stamp) {
  proto::VName anchor_name = StampedVNameFromRange(source_range, stamp);
  if (stamped_ranges_.emplace(source_range, stamp).second) {
    UnconditionalRecordRange(anchor_name, source_range);
  }
  recorder_->AddEdge(VNameRef(anchor_name), anchor_edge_kind,
                     VNameRefFromNodeId(primary_anchored_to_decl));
  ApplyMetadataRules(source_range, primary_anchored_to_decl,
                     primary_anchored_to_def, anchor_edge_kind, anchor_name);
}

void KytheGraphObserver::RecordAnchor(
    const GraphObserver::Range& source_range,
    const GraphObserver::NodeId& primary_anchored_to,
    EdgeKindID anchor_edge_kind, Claimability cl) {
  CHECK(!file_stack_.empty());
  if (drop_redundant_wraiths_ &&
      !range_edges_
           .insert(RangeEdge{
               source_range.PhysicalRange, anchor_edge_kind,
               primary_anchored_to,
               RangeEdge::ComputeHash(source_range.PhysicalRange,
                                      anchor_edge_kind, primary_anchored_to)})
           .second) {
    return;
  }
  proto::VName anchor_name = VNameFromRange(source_range);
  if (claimRange(source_range) || claimNode(primary_anchored_to)) {
    RecordRange(anchor_name, source_range);
    cl = Claimability::Unclaimable;
  }
  if (cl == Claimability::Unclaimable) {
    recorder_->AddEdge(VNameRef(anchor_name), anchor_edge_kind,
                       VNameRefFromNodeId(primary_anchored_to));
    ApplyMetadataRules(source_range, primary_anchored_to, std::nullopt,
                       anchor_edge_kind, anchor_name);
  }
}

void KytheGraphObserver::RecordAnchor(
    const GraphObserver::Range& source_range,
    const kythe::proto::VName& primary_anchored_to, EdgeKindID anchor_edge_kind,
    Claimability cl) {
  CHECK(!file_stack_.empty());
  proto::VName anchor_name = VNameFromRange(source_range);
  if (claimRange(source_range)) {
    RecordRange(anchor_name, source_range);
    cl = Claimability::Unclaimable;
  }
  if (cl == Claimability::Unclaimable) {
    recorder_->AddEdge(VNameRef(anchor_name), anchor_edge_kind,
                       VNameRef(primary_anchored_to));
  }
}

void KytheGraphObserver::recordCallEdge(
    const GraphObserver::Range& source_range, const NodeId& caller_id,
    const NodeId& callee_id, Implicit i, CallDispatch d) {
  RecordAnchor(source_range, caller_id, EdgeKindID::kChildOf,
               Claimability::Claimable);
  EdgeKindID kind;
  if (i == Implicit::Yes) {
    kind = d == CallDispatch::kDirect ? EdgeKindID::kRefCallDirectImplicit
                                      : EdgeKindID::kRefCallImplicit;
  } else {
    kind = d == CallDispatch::kDirect ? EdgeKindID::kRefCallDirect
                                      : EdgeKindID::kRefCall;
  }
  RecordAnchor(source_range, callee_id, kind, Claimability::Unclaimable);
}

std::optional<GraphObserver::NodeId> KytheGraphObserver::recordFileInitializer(
    const Range& range) {
  // Always use physical ranges for these. We'll record a reference site as
  // well, but including virtual ranges in the callgraph itself isn't useful
  // (since the data we'd be expressing amounts to 'a call exists, somewhere').
  if (range.Kind == GraphObserver::Range::RangeKind::Implicit ||
      !range.PhysicalRange.getBegin().isValid()) {
    return std::nullopt;
  }
  clang::SourceLocation begin = range.PhysicalRange.getBegin();
  if (begin.isMacroID()) {
    begin = SourceManager->getExpansionLoc(begin);
  }
  SourceLocation file_start =
      SourceManager->getLocForStartOfFile(SourceManager->getFileID(begin));
  const auto* token = getClaimTokenForLocation(file_start);
  NodeId file_id = MakeNodeId(token, "#init");
  if (recorded_inits_.insert(token).second) {
    MarkedSource file_source;
    file_source.set_pre_text(token->vname().path());
    file_source.set_kind(MarkedSource::IDENTIFIER);
    recordFunctionNode(file_id, Completeness::Definition,
                       FunctionSubkind::Initializer, file_source);
    recordDefinitionBindingRange(
        Range(clang::SourceRange(file_start, file_start), token), file_id,
        std::nullopt);
  }
  return file_id;
}

VNameRef KytheGraphObserver::VNameRefFromNodeId(
    const GraphObserver::NodeId& node_id) const {
  if (node_id.getToken() == getVNameClaimToken()) {
    return DecodeMintedVName(node_id);
  }
  VNameRef out_ref;
  out_ref.set_language(absl::string_view(supported_language::kIndexerLang));
  if (const auto* token =
          clang::dyn_cast<KytheClaimToken>(node_id.getToken())) {
    token->DecorateVName(&out_ref);
    if (token->language_independent()) {
      out_ref.set_language(absl::string_view());
    }
  }
  out_ref.set_signature(ConvertRef(node_id.IdentityRef()));
  return out_ref;
}

void KytheGraphObserver::recordParamEdge(const NodeId& param_of_id,
                                         uint32_t ordinal,
                                         const NodeId& param_id) {
  recorder_->AddEdge(VNameRefFromNodeId(param_of_id), EdgeKindID::kParam,
                     VNameRefFromNodeId(param_id), ordinal);
}

void KytheGraphObserver::recordTParamEdge(const NodeId& param_of_id,
                                          uint32_t ordinal,
                                          const NodeId& param_id) {
  recorder_->AddEdge(VNameRefFromNodeId(param_of_id), EdgeKindID::kTParam,
                     VNameRefFromNodeId(param_id), ordinal);
}

void KytheGraphObserver::recordChildOfEdge(const NodeId& child_id,
                                           const NodeId& parent_id) {
  recorder_->AddEdge(VNameRefFromNodeId(child_id), EdgeKindID::kChildOf,
                     VNameRefFromNodeId(parent_id));
}

void KytheGraphObserver::recordTypeEdge(const NodeId& term_id,
                                        const NodeId& type_id) {
  recorder_->AddEdge(VNameRefFromNodeId(term_id), EdgeKindID::kHasType,
                     VNameRefFromNodeId(type_id));
}

void KytheGraphObserver::recordInfluences(const NodeId& influencer,
                                          const NodeId& influenced) {
  recorder_->AddEdge(VNameRefFromNodeId(influencer), EdgeKindID::kInfluences,
                     VNameRefFromNodeId(influenced));
}

void KytheGraphObserver::recordUpperBoundEdge(const NodeId& TypeNodeId,
                                              const NodeId& TypeBoundNodeId) {
  recorder_->AddEdge(VNameRefFromNodeId(TypeNodeId), EdgeKindID::kBoundedUpper,
                     VNameRefFromNodeId(TypeBoundNodeId));
}

void KytheGraphObserver::recordVariance(const NodeId& TypeNodeId,
                                        const Variance V) {
  std::string Variance;
  switch (V) {
    // KytheGraphObserver prefix shouldn't be necessary but some compiler
    // incantations complain if it is not there.
    case KytheGraphObserver::Variance::Contravariant:
      Variance = "contravariant";
      break;
    case KytheGraphObserver::Variance::Covariant:
      Variance = "covariant";
      break;
    case KytheGraphObserver::Variance::Invariant:
      Variance = "invariant";
      break;
  }
  recorder_->AddProperty(VNameRefFromNodeId(TypeNodeId), PropertyID::kVariance,
                         Variance);
}

void KytheGraphObserver::recordSpecEdge(const NodeId& term_id,
                                        const NodeId& type_id,
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

void KytheGraphObserver::recordInstEdge(const NodeId& term_id,
                                        const NodeId& type_id,
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

void KytheGraphObserver::recordOverridesEdge(const NodeId& overrider,
                                             const NodeId& base_object) {
  recorder_->AddEdge(VNameRefFromNodeId(overrider), EdgeKindID::kOverrides,
                     VNameRefFromNodeId(base_object));
}

void KytheGraphObserver::recordOverridesRootEdge(const NodeId& overrider,
                                                 const NodeId& root_object) {
  recorder_->AddEdge(VNameRefFromNodeId(overrider), EdgeKindID::kOverridesRoot,
                     VNameRefFromNodeId(root_object));
}

GraphObserver::NodeId KytheGraphObserver::nodeIdForTypeAliasNode(
    const NameId& alias_name, const NodeId& aliased_type) const {
  return MakeNodeId(&type_token_, "talias(" + alias_name.ToString() + "," +
                                      aliased_type.ToClaimedString() + ")");
}

GraphObserver::NodeId KytheGraphObserver::recordTypeAliasNode(
    const NodeId& type_id, const NodeId& aliased_type,
    const std::optional<NodeId>& root_aliased_type,
    const std::optional<MarkedSource>& marked_source) {
  if (!deferring_nodes_ ||
      written_types_.insert(type_id.ToClaimedString()).second) {
    VNameRef type_vname(VNameRefFromNodeId(type_id));
    recorder_->AddProperty(type_vname, NodeKindID::kTAlias);
    AddMarkedSource(type_vname, marked_source);
    VNameRef aliased_type_vname(VNameRefFromNodeId(aliased_type));
    recorder_->AddEdge(type_vname, EdgeKindID::kAliases,
                       VNameRef(aliased_type_vname));
    if (root_aliased_type) {
      VNameRef root_aliased_type_vname(VNameRefFromNodeId(*root_aliased_type));
      recorder_->AddEdge(type_vname, EdgeKindID::kAliasesRoot,
                         VNameRef(root_aliased_type_vname));
    }
  }
  return type_id;
}

void KytheGraphObserver::recordDocumentationText(
    const NodeId& node, const std::string& doc_text,
    const std::vector<NodeId>& doc_links) {
  std::string signature = doc_text;
  for (const auto& link : doc_links) {
    signature.push_back(',');
    signature.append(link.ToClaimedString());
  }
  // Force hashing because the serving backend gets upset if certain
  // characters appear in VName fields.
  NodeId doc_id = MakeNodeId(node.getToken(), ForceEncodeString(signature));
  VNameRef doc_vname(VNameRefFromNodeId(doc_id));
  if (written_docs_.insert(doc_id.ToClaimedString()).second) {
    recorder_->AddProperty(doc_vname, NodeKindID::kDoc);
    recorder_->AddProperty(doc_vname, PropertyID::kText, doc_text);
    size_t param_index = 0;
    for (const auto& link : doc_links) {
      recorder_->AddEdge(doc_vname, EdgeKindID::kParam,
                         VNameRefFromNodeId(link), param_index++);
    }
  }
  recorder_->AddEdge(doc_vname, EdgeKindID::kDocuments,
                     VNameRefFromNodeId(node));
}

void KytheGraphObserver::recordDocumentationRange(
    const GraphObserver::Range& source_range, const NodeId& node) {
  RecordAnchor(source_range, node, EdgeKindID::kDocuments,
               Claimability::Claimable);
}

void KytheGraphObserver::recordFullDefinitionRange(
    const GraphObserver::Range& source_range, const NodeId& node_decl,
    const std::optional<NodeId>& node_def) {
  RecordStampedAnchor(source_range, node_decl, node_def,
                      EdgeKindID::kDefinesFull, node_decl);
}

void KytheGraphObserver::recordDefinitionBindingRange(
    const GraphObserver::Range& binding_range, const NodeId& node_decl,
    const std::optional<NodeId>& node_def, Stamping stamping) {
  if (stamping == Stamping::Stamped) {
    RecordStampedAnchor(binding_range, node_decl, node_def,
                        EdgeKindID::kDefinesBinding, node_decl);
    return;
  }
  RecordAnchor(binding_range, node_decl, EdgeKindID::kDefinesBinding,
               Claimability::Unclaimable);
}

void KytheGraphObserver::recordDefinitionRangeWithBinding(
    const GraphObserver::Range& source_range,
    const GraphObserver::Range& binding_range, const NodeId& node_decl,
    const std::optional<NodeId>& node_def) {
  RecordStampedAnchor(source_range, node_decl, node_def,
                      EdgeKindID::kDefinesFull, node_decl);
  RecordStampedAnchor(binding_range, node_decl, node_def,
                      EdgeKindID::kDefinesBinding, node_decl);
}

void KytheGraphObserver::recordCompletion(const NodeId& node,
                                          const NodeId& completing_node) {
  recorder_->AddEdge(VNameRefFromNodeId(node), EdgeKindID::kCompletedby,
                     VNameRefFromNodeId(completing_node));
}

GraphObserver::NodeId KytheGraphObserver::nodeIdForNominalTypeNode(
    const NameId& name_id) const {
  // Appending #t to a name produces the VName signature of the nominal
  // type node referring to that name. For example, the VName for a
  // forward-declared class type will look like "C#c#t".
  return MakeNodeId(&type_token_, name_id.ToString() + "#t");
}

GraphObserver::NodeId KytheGraphObserver::recordNominalTypeNode(
    const NodeId& name_id, const std::optional<MarkedSource>& marked_source,
    const std::optional<NodeId>& parent) {
  if (!deferring_nodes_ ||
      written_types_.insert(name_id.ToClaimedString()).second) {
    VNameRef type_vname(VNameRefFromNodeId(name_id));
    AddMarkedSource(type_vname, marked_source);
    recorder_->AddProperty(type_vname, NodeKindID::kTNominal);
    if (parent) {
      recorder_->AddEdge(type_vname, EdgeKindID::kChildOf,
                         VNameRefFromNodeId(*parent));
    }
  }
  return name_id;
}

GraphObserver::NodeId KytheGraphObserver::nodeIdForTsigmaNode(
    absl::Span<const NodeId> params) const {
  return MakeNodeId(
      &type_token_,
      absl::StrCat("#sigma(",
                   absl::StrJoin(params, ",", ClaimedStringFormatter{}), ")"));
}

GraphObserver::NodeId KytheGraphObserver::recordTsigmaNode(
    const NodeId& tsigma_id, absl::Span<const NodeId> params) {
  if (!deferring_nodes_ ||
      written_types_.insert(tsigma_id.ToClaimedString()).second) {
    VNameRef tsigma_vname = VNameRefFromNodeId(tsigma_id);
    recorder_->AddProperty(tsigma_vname, NodeKindID::kTSigma);
    for (uint32_t param_index = 0; param_index < params.size(); ++param_index) {
      recorder_->AddEdge(tsigma_vname, EdgeKindID::kParam,
                         VNameRefFromNodeId(params[param_index]), param_index);
    }
  }
  return tsigma_id;
}

GraphObserver::NodeId KytheGraphObserver::nodeIdForTappNode(
    const NodeId& tycon_id, absl::Span<const NodeId> params) const {
  // We can't just use juxtaposition here because it leads to ambiguity
  // as we can't assume that we have kind information, eg
  //   foo bar baz
  // might be
  //   foo (bar baz)
  // We'll turn it into a C-style function application:
  //   foo(bar,baz) || foo(bar(baz))
  return MakeNodeId(
      &type_token_,
      absl::StrCat(tycon_id.ToClaimedString(), "(",
                   absl::StrJoin(params, ",", ClaimedStringFormatter{}), ")"));
}

GraphObserver::NodeId KytheGraphObserver::recordTappNode(
    const NodeId& tapp_id, const NodeId& tycon_id,
    absl::Span<const NodeId> params, unsigned first_default_param) {
  CHECK(first_default_param <= params.size());
  if (!deferring_nodes_ ||
      written_types_.insert(tapp_id.ToClaimedString()).second) {
    VNameRef tapp_vname = VNameRefFromNodeId(tapp_id);
    recorder_->AddProperty(tapp_vname, NodeKindID::kTApp);
    if (first_default_param < params.size()) {
      recorder_->AddProperty(tapp_vname, PropertyID::kParamDefault,
                             first_default_param);
    }
    recorder_->AddEdge(tapp_vname, EdgeKindID::kParam,
                       VNameRefFromNodeId(tycon_id), 0);
    for (uint32_t param_index = 0; param_index < params.size(); ++param_index) {
      recorder_->AddEdge(tapp_vname, EdgeKindID::kParam,
                         VNameRefFromNodeId(params[param_index]),
                         param_index + 1);
    }
  }
  return tapp_id;
}

void KytheGraphObserver::recordEnumNode(const NodeId& node_id,
                                        Completeness completeness,
                                        EnumKind enum_kind) {
  VNameRef node_vname = VNameRefFromNodeId(node_id);
  recorder_->AddProperty(node_vname, NodeKindID::kSum);
  recorder_->AddProperty(node_vname, PropertyID::kComplete,
                         CompletenessToString(completeness));
  recorder_->AddProperty(node_vname, PropertyID::kSubkind,
                         enum_kind == EnumKind::Scoped ? "enumClass" : "enum");
}

void KytheGraphObserver::recordIntegerConstantNode(const NodeId& node_id,
                                                   const llvm::APSInt& Value) {
  VNameRef node_vname(VNameRefFromNodeId(node_id));
  recorder_->AddProperty(node_vname, NodeKindID::kConstant);
  recorder_->AddProperty(node_vname, PropertyID::kText,
                         llvm::toString(Value, 10));
}

void KytheGraphObserver::recordFunctionNode(
    const NodeId& node_id, Completeness completeness, FunctionSubkind subkind,
    const std::optional<MarkedSource>& marked_source) {
  VNameRef node_vname = VNameRefFromNodeId(node_id);
  recorder_->AddProperty(node_vname, NodeKindID::kFunction);
  recorder_->AddProperty(node_vname, PropertyID::kComplete,
                         CompletenessToString(completeness));
  AddMarkedSource(node_vname, marked_source);
  if (subkind != FunctionSubkind::None) {
    recorder_->AddProperty(node_vname, PropertyID::kSubkind,
                           FunctionSubkindToString(subkind));
  }
}

void KytheGraphObserver::assignUsr(const NodeId& node, llvm::StringRef usr,
                                   int byte_size) {
  if (byte_size < 0) return;
  auto hash = llvm::SHA1::hash(llvm::arrayRefFromStringRef(usr));
  auto hex = llvm::toHex(
      llvm::StringRef(reinterpret_cast<const char*>(hash.data()),
                      std::min(hash.size(), static_cast<size_t>(byte_size))));
  VNameRef node_vname = VNameRefFromNodeId(node);
  VNameRef usr_vname;
  usr_vname.set_corpus(usr_default_corpus_
                           ? absl::string_view(default_token_.vname().corpus())
                           : "");
  usr_vname.set_signature(hex);
  usr_vname.set_language("usr");
  recorder_->AddProperty(usr_vname, NodeKindID::kClangUsr);
  recorder_->AddEdge(usr_vname, EdgeKindID::kClangUsr, node_vname);
}

void KytheGraphObserver::recordMarkedSource(
    const NodeId& node_id, const std::optional<MarkedSource>& marked_source) {
  VNameRef node_vname = VNameRefFromNodeId(node_id);
  AddMarkedSource(node_vname, marked_source);
}

void KytheGraphObserver::recordTVarNode(
    const NodeId& node_id, const std::optional<MarkedSource>& marked_source) {
  auto node_vname = VNameRefFromNodeId(node_id);
  recorder_->AddProperty(node_vname, NodeKindID::kTVar);
  AddMarkedSource(node_vname, marked_source);
}

void KytheGraphObserver::recordLookupNode(const NodeId& node_id,
                                          llvm::StringRef text) {
  VNameRef node_vname = VNameRefFromNodeId(node_id);
  recorder_->AddProperty(node_vname, NodeKindID::kLookup);
  recorder_->AddProperty(node_vname, PropertyID::kText, ConvertRef(text));
  MarkedSource marked_source;
  marked_source.set_kind(MarkedSource::BOX);
  auto* lhs = marked_source.add_child();
  lhs->set_kind(MarkedSource::CONTEXT);
  lhs = lhs->add_child();
  lhs->set_kind(MarkedSource::PARAMETER_LOOKUP_BY_PARAM);
  lhs->set_pre_text("dependent(");
  lhs->set_post_child_text("::");
  lhs->set_post_text(")::");
  auto* rhs = marked_source.add_child();
  rhs->set_kind(MarkedSource::IDENTIFIER);
  rhs->set_pre_text(text.str());
  recorder_->AddMarkedSource(node_vname, marked_source);
}

void KytheGraphObserver::recordInterfaceNode(
    const NodeId& node_id, const std::optional<MarkedSource>& marked_source) {
  VNameRef node_vname = VNameRefFromNodeId(node_id);
  recorder_->AddProperty(node_vname, NodeKindID::kInterface);
  AddMarkedSource(node_vname, marked_source);
}

void KytheGraphObserver::recordRecordNode(
    const NodeId& node_id, RecordKind kind, Completeness completeness,
    const std::optional<MarkedSource>& marked_source) {
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
    case RecordKind::Category:
      recorder_->AddProperty(node_vname, PropertyID::kSubkind, "category");
      break;
  }
  recorder_->AddProperty(node_vname, PropertyID::kComplete,
                         CompletenessToString(completeness));
  AddMarkedSource(node_vname, marked_source);
}

void KytheGraphObserver::recordTypeSpellingLocation(
    const GraphObserver::Range& type_source_range, const NodeId& type_id,
    Claimability claimability, Implicit i) {
  RecordAnchor(type_source_range, type_id,
               i == Implicit::Yes ? EdgeKindID::kRefImplicit : EdgeKindID::kRef,
               claimability);
}

void KytheGraphObserver::recordTypeIdSpellingLocation(
    const GraphObserver::Range& type_source_range, const NodeId& type_id,
    Claimability claimability, Implicit i) {
  RecordAnchor(
      type_source_range, type_id,
      i == Implicit::Yes ? EdgeKindID::kRefImplicit : EdgeKindID::kRefId,
      claimability);
}

void KytheGraphObserver::recordCategoryExtendsEdge(const NodeId& from,
                                                   const NodeId& to) {
  recorder_->AddEdge(VNameRefFromNodeId(from), EdgeKindID::kExtendsCategory,
                     VNameRefFromNodeId(to));
}

void KytheGraphObserver::recordExtendsEdge(const NodeId& from, const NodeId& to,
                                           bool is_virtual,
                                           clang::AccessSpecifier specifier) {
  switch (specifier) {
    case clang::AccessSpecifier::AS_public:
      recorder_->AddEdge(VNameRefFromNodeId(from),
                         is_virtual ? EdgeKindID::kExtendsPublicVirtual
                                    : EdgeKindID::kExtendsPublic,
                         VNameRefFromNodeId(to));
      return;
    case clang::AccessSpecifier::AS_protected:
      recorder_->AddEdge(VNameRefFromNodeId(from),
                         is_virtual ? EdgeKindID::kExtendsProtectedVirtual
                                    : EdgeKindID::kExtendsProtected,
                         VNameRefFromNodeId(to));
      return;
    case clang::AccessSpecifier::AS_private:
      recorder_->AddEdge(VNameRefFromNodeId(from),
                         is_virtual ? EdgeKindID::kExtendsPrivateVirtual
                                    : EdgeKindID::kExtendsPrivate,
                         VNameRefFromNodeId(to));
      return;
    case clang::AccessSpecifier::AS_none:
      recorder_->AddEdge(
          VNameRefFromNodeId(from),
          is_virtual ? EdgeKindID::kExtendsVirtual : EdgeKindID::kExtends,
          VNameRefFromNodeId(to));
      return;
  }
}

void KytheGraphObserver::recordDeclUseLocationInDocumentation(
    const GraphObserver::Range& source_range, const NodeId& node) {
  RecordAnchor(source_range, node, EdgeKindID::kRefDoc,
               Claimability::Claimable);
}

void KytheGraphObserver::recordDeclUseLocation(
    const GraphObserver::Range& source_range, const NodeId& node,
    Claimability claimability, Implicit i) {
  RecordAnchor(source_range, node,
               i == Implicit::Yes ? EdgeKindID::kRefImplicit : EdgeKindID::kRef,
               claimability);
}

void KytheGraphObserver::recordBlameLocation(
    const GraphObserver::Range& source_range, const NodeId& blame,
    Claimability claimability, Implicit i) {
  RecordAnchor(source_range, blame, EdgeKindID::kChildOf,
               Claimability::Claimable);
}

void KytheGraphObserver::recordSemanticDeclUseLocation(
    const GraphObserver::Range& source_range, const NodeId& node, UseKind kind,
    Claimability claimability, Implicit i) {
  if (kind == GraphObserver::UseKind::kUnknown ||
      kind == GraphObserver::UseKind::kReadWrite ||
      kind == GraphObserver::UseKind::kTakeAlias) {
    auto out_kind =
        (i == GraphObserver::Implicit::Yes ? EdgeKindID::kRefImplicit
                                           : EdgeKindID::kRef);
    RecordAnchor(source_range, node, out_kind, claimability);
  }
  if (kind == GraphObserver::UseKind::kWrite ||
      kind == GraphObserver::UseKind::kReadWrite) {
    auto out_kind =
        (i == GraphObserver::Implicit::Yes ? EdgeKindID::kRefWritesImplicit
                                           : EdgeKindID::kRefWrites);
    RecordAnchor(source_range, node, out_kind, claimability);
  }
}

void KytheGraphObserver::recordInitLocation(
    const GraphObserver::Range& source_range, const NodeId& node,
    Claimability claimability, Implicit i) {
  RecordAnchor(
      source_range, node,
      i == Implicit::Yes ? EdgeKindID::kRefInitImplicit : EdgeKindID::kRefInit,
      claimability);
}

void KytheGraphObserver::recordStaticVariable(const NodeId& VarNodeId) {
  const VNameRef node_vname = VNameRefFromNodeId(VarNodeId);
  recorder_->AddProperty(node_vname, PropertyID::kTagStatic, "");
}

void KytheGraphObserver::recordVisibility(const NodeId& FieldNodeId,
                                          clang::AccessSpecifier access) {
  if (access == clang::AS_none) {
    return;
  }
  const VNameRef node_vname = VNameRefFromNodeId(FieldNodeId);
  recorder_->AddProperty(node_vname, PropertyID::kVisibility,
                         VisibilityPropertyValue(access));
}

void KytheGraphObserver::recordDeprecated(const NodeId& NodeId,
                                          llvm::StringRef Advice) {
  const VNameRef node_vname = VNameRefFromNodeId(NodeId);
  recorder_->AddProperty(node_vname, PropertyID::kTagDeprecated,
                         ConvertRef(Advice));
}

void KytheGraphObserver::recordDiagnostic(const Range& Range,
                                          llvm::StringRef Signature,
                                          llvm::StringRef Message) {
  proto::VName anchor_vname = VNameFromRange(Range);

  proto::VName dn_vname;
  dn_vname.set_signature(
      absl::StrCat(anchor_vname.signature(), "-", ConvertRef(Signature)));
  dn_vname.set_corpus(anchor_vname.corpus());
  dn_vname.set_root(anchor_vname.root());
  dn_vname.set_path(anchor_vname.path());
  dn_vname.set_language(anchor_vname.language());

  recorder_->AddProperty(VNameRef(dn_vname), NodeKindID::kDiagnostic);
  recorder_->AddProperty(VNameRef(dn_vname), PropertyID::kDiagnosticMessage,
                         ConvertRef(Message));

  recorder_->AddEdge(VNameRef(anchor_vname), EdgeKindID::kTagged,
                     VNameRef(dn_vname));
}

GraphObserver::NodeId KytheGraphObserver::getNodeIdForBuiltinType(
    llvm::StringRef spelling) const {
  const auto& info = builtins_.find(spelling.str());
  if (info == builtins_.end()) {
    if (absl::GetFlag(FLAGS_fail_on_unimplemented_builtin)) {
      LOG(FATAL) << "Missing builtin " << spelling.str();
    }
    DLOG(LEVEL(-1)) << "Missing builtin " << spelling.str();
    MarkedSource sig;
    sig.set_kind(MarkedSource::IDENTIFIER);
    sig.set_pre_text(std::string(spelling));
    builtins_.emplace(spelling.str(), Builtin{NodeId::CreateUncompressed(
                                                  getDefaultClaimToken(),
                                                  spelling.str() + "#builtin"),
                                              sig, true});
    auto* new_builtin = &builtins_.find(spelling.str())->second;
    EmitBuiltin(new_builtin);
    return new_builtin->node_id;
  }
  if (!info->second.emitted) {
    EmitBuiltin(&info->second);
  }
  return info->second.node_id;
}

void KytheGraphObserver::applyMetadataFile(
    clang::FileID id, const clang::FileEntry* file,
    const std::string& search_string, const clang::FileEntry* target_file) {
  const std::optional<llvm::MemoryBufferRef> buffer =
      SourceManager->getMemoryBufferForFileOrNone(file);
  if (!buffer) {
    absl::FPrintF(stderr, "Couldn't get content for %s\n",
                  file->getName().str());
    return;
  }
  const std::optional<llvm::MemoryBufferRef> target_buffer =
      SourceManager->getMemoryBufferForFileOrNone(target_file);
  if (!target_buffer) {
    absl::FPrintF(stderr, "Couldn't get content for %s\n",
                  target_file->getName().str());
    return;
  }
  if (auto metadata = meta_supports_->ParseFile(
          std::string(file->getName()),
          absl::string_view(buffer->getBuffer().data(),
                            buffer->getBufferSize()),
          search_string,
          absl::string_view(target_buffer->getBuffer().data(),
                            target_buffer->getBufferSize()))) {
    meta_.emplace(id, std::move(metadata));
  }
}

void KytheGraphObserver::AppendMainSourceFileIdentifierToStream(
    llvm::raw_ostream& ostream) const {
  if (main_source_file_token_) {
    AppendRangeToStream(ostream,
                        Range(main_source_file_loc_, main_source_file_token_));
  }
}

bool KytheGraphObserver::isMainSourceFileRelatedLocation(
    clang::SourceLocation location) const {
  // Where was this thing spelled out originally?
  if (location.isInvalid()) {
    return true;
  }
  if (!location.isFileID()) {
    location = SourceManager->getExpansionLoc(location);
    if (!location.isValid() || !location.isFileID()) {
      return true;
    }
  }
  clang::FileID file = SourceManager->getFileID(location);
  if (file.isInvalid()) {
    return true;
  }
  if (const clang::FileEntry* entry = SourceManager->getFileEntryForID(file)) {
    return transitively_reached_through_header_.find(entry->getUniqueID()) ==
           transitively_reached_through_header_.end();
  }
  return true;
}

bool KytheGraphObserver::claimImplicitNode(const std::string& identifier) {
  kythe::proto::VName node_vname;
  node_vname.set_signature(identifier);
  return client_->Claim(claimant_, node_vname);
}

void KytheGraphObserver::finishImplicitNode(const std::string& identifier) {
  // TODO(zarko): Handle this in two phases. This should commmit the claim.
}

bool KytheGraphObserver::claimBatch(
    std::vector<std::pair<std::string, bool>>* pairs) {
  return client_->ClaimBatch(pairs);
}

void KytheGraphObserver::pushFile(clang::SourceLocation blame_location,
                                  clang::SourceLocation source_location) {
  PreprocessorContext previous_context =
      file_stack_.empty() ? starting_context_ : file_stack_.back().context;
  bool has_previous_uid = !file_stack_.empty();
  llvm::sys::fs::UniqueID previous_uid;
  bool in_header = false;
  if (has_previous_uid) {
    previous_uid = file_stack_.back().uid;
    in_header = transitively_reached_through_header_.find(previous_uid) !=
                transitively_reached_through_header_.end();
  }
  file_stack_.push_back(FileState{});
  FileState& state = file_stack_.back();
  state.claimed = true;
  if (source_location.isValid()) {
    if (source_location.isMacroID()) {
      source_location = SourceManager->getExpansionLoc(source_location);
    }
    CHECK(source_location.isFileID());
    clang::FileID file = SourceManager->getFileID(source_location);
    if (file.isInvalid()) {
      // An actually invalid location.
    } else {
      const clang::FileEntry* entry = SourceManager->getFileEntryForID(file);
      if (entry) {
        // An actual file.
        state.vname = state.base_vname = VNameFromFileEntry(entry);
        state.uid = entry->getUniqueID();
        // TODO(zarko): If modules are enabled, check there to see whether
        // `entry` is a textual header.
        if (in_header ||
            (has_previous_uid &&
             !llvm::StringRef(entry->getName()).endswith(".inc"))) {
          transitively_reached_through_header_.insert(state.uid);
        }
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
                absl::FPrintF(
                    stderr,
                    "Warning: when looking for %s[%s]:%u: missing source "
                    "offset\n",
                    vfs_->get_debug_uid_string(previous_uid), previous_context,
                    offset);
              }
            } else {
              absl::FPrintF(
                  stderr,
                  "Warning: when looking for %s[%s]:%u: missing source "
                  "context\n",
                  vfs_->get_debug_uid_string(previous_uid), previous_context,
                  offset);
            }
          } else {
            absl::FPrintF(
                stderr,
                "Warning: when looking for %s[%s]:%u: missing source path\n",
                vfs_->get_debug_uid_string(previous_uid), previous_context,
                offset);
          }
        }
        state.vname.set_signature(absl::StrCat(
            state.context, state.vname.signature(), build_config_));
        if (client_->Claim(claimant_, state.vname)) {
          if (recorded_files_.insert(entry).second) {
            const std::optional<llvm::MemoryBufferRef> buf =
                SourceManager->getMemoryBufferForFileOrNone(entry);
            if (!buf) {
              // TODO(zarko): diagnostic logging.
            } else {
              recorder_->AddFileContent(VNameRef(state.base_vname),
                                        ConvertRef(buf->getBuffer()));
            }
          }
        } else {
          state.claimed = false;
        }
        KytheClaimToken token;
        token.set_vname(state.vname);
        token.set_rough_claimed(state.claimed);
        claim_checked_files_.emplace(file, token);
        if (state.claimed) {
          KytheClaimToken file_token;
          file_token.set_vname(state.vname);
          file_token.set_rough_claimed(state.claimed);
          file_token.set_language_independent(true);
          claimed_file_specific_tokens_.emplace(file, file_token);
        }
        if (!has_previous_uid) {
          main_source_file_loc_ = source_location;
          main_source_file_token_ = &claim_checked_files_[file];
        }
      } else {
        // A builtin location.
      }
    }
  }
}

void KytheGraphObserver::popFile() {
  CHECK(!file_stack_.empty());
  FileState state = file_stack_.back();
  file_stack_.pop_back();
  if (file_stack_.empty()) {
    deferred_anchors_.clear();
  }
}

void KytheGraphObserver::iterateOverClaimedFiles(
    std::function<bool(clang::FileID, const NodeId&)> iter) const {
  for (const auto& file : claimed_file_specific_tokens_) {
    if (!iter(file.first, MakeNodeId(&file.second, ""))) {
      return;
    }
  }
}

bool KytheGraphObserver::claimRange(const GraphObserver::Range& range) {
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
  CHECK(source_location.isFileID());
  clang::FileID file = SourceManager->getFileID(source_location);
  if (file.isInvalid()) {
    return true;
  }
  auto token = claim_checked_files_.find(file);
  return token != claim_checked_files_.end() ? token->second.rough_claimed()
                                             : false;
}

void KytheGraphObserver::AddContextInformation(
    const std::string& path, const PreprocessorContext& context,
    unsigned offset, const PreprocessorContext& dest_context) {
  auto found_file = vfs_->status(path);
  if (found_file) {
    path_to_context_data_[found_file->getUniqueID()][context][offset] =
        dest_context;
  } else {
    absl::FPrintF(stderr,
                  "WARNING: Path %s could not be mapped to a VFS record.\n",
                  path);
  }
}

const KytheClaimToken* KytheGraphObserver::getClaimTokenForLocation(
    clang::SourceLocation source_location) const {
  if (!source_location.isValid()) {
    return &default_token_;
  }
  if (source_location.isMacroID()) {
    source_location = SourceManager->getExpansionLoc(source_location);
  }
  CHECK(source_location.isFileID());
  clang::FileID file = SourceManager->getFileID(source_location);
  if (file.isInvalid()) {
    return &default_token_;
  }
  auto token = claim_checked_files_.find(file);
  if (token == claim_checked_files_.end()) {
    if (SourceManager->isLoadedFileID(file)) {
      // This is the first time we've encountered a file loaded from a pch/pcm.
      if (auto* entry = SourceManager->getFileEntryForID(file)) {
        auto vname = VNameFromFileEntry(entry);
        KytheClaimToken new_token;
        new_token.set_vname(VNameFromFileEntry(entry));
        new_token.set_rough_claimed(false);
        token = claim_checked_files_.emplace(file, new_token).first;
      }
    }
  }
  return token != claim_checked_files_.end() ? &token->second : &default_token_;
}

const KytheClaimToken* KytheGraphObserver::getClaimTokenForRange(
    const clang::SourceRange& range) const {
  return getClaimTokenForLocation(range.getBegin());
}

const KytheClaimToken* KytheGraphObserver::getAnonymousNamespaceClaimToken(
    clang::SourceLocation loc) const {
  if (isMainSourceFileRelatedLocation(loc)) {
    CHECK(main_source_file_token_ != nullptr);
    return main_source_file_token_;
  }
  return &getNamespaceTokens(loc).anonymous;
}

const KytheClaimToken* KytheGraphObserver::getNamespaceClaimToken(
    clang::SourceLocation loc) const {
  return &getNamespaceTokens(loc).named;
}

const KytheGraphObserver::NamespaceTokens&
KytheGraphObserver::getNamespaceTokens(clang::SourceLocation loc) const {
  auto* file_token = getClaimTokenForLocation(loc);
  auto [iter, inserted] =
      namespace_tokens_.emplace(file_token, NamespaceTokens{});
  if (inserted) {
    // Named namespaces belong to the same corpus as structural types due to
    // their use as extension points which may be opened from a file in any
    // corpus, but should still refer to the same node.
    iter->second.named.mutable_vname()->set_corpus(
        type_token_.vname().corpus());
    iter->second.named.set_rough_claimed(file_token->rough_claimed());
    // Anonymous namespaces are unique to the translation in which they're
    // defined, which we approximate by using the file's corpus.
    iter->second.anonymous.mutable_vname()->set_corpus(
        file_token->vname().corpus());
    iter->second.anonymous.set_rough_claimed(file_token->rough_claimed());
  }
  return iter->second;
}

void KytheGraphObserver::RegisterBuiltins() {
  auto RegisterBuiltin = [&](absl::string_view name,
                             const MarkedSource& marked_source) {
    builtins_.emplace(name, Builtin{NodeId::CreateUncompressed(
                                        getDefaultClaimToken(),
                                        absl::StrCat(name, "#builtin")),
                                    marked_source, false});
  };
  auto RegisterTokenBuiltin = [&](absl::string_view name,
                                  absl::string_view token) {
    MarkedSource sig;
    sig.set_kind(MarkedSource::IDENTIFIER);
    sig.set_pre_text(token);
    RegisterBuiltin(name, sig);
  };

  for (absl::string_view token : kUniformClangBuiltins) {
    RegisterTokenBuiltin(token, token);
  }

  RegisterTokenBuiltin("std::nullptr_t", "nullptr_t");
  RegisterTokenBuiltin("<dependent type>", "dependent");
  RegisterTokenBuiltin("knrfn", "function");

  {
    MarkedSource lhs_tycon_builtin;
    auto* lhs_tycon = lhs_tycon_builtin.add_child();
    auto* lookup = lhs_tycon_builtin.add_child();
    lookup->set_kind(MarkedSource::LOOKUP_BY_PARAM);
    lookup->set_lookup_index(1);
    lhs_tycon->set_kind(MarkedSource::IDENTIFIER);
    lhs_tycon->set_pre_text("const ");
    RegisterBuiltin("const", lhs_tycon_builtin);
    lhs_tycon->set_pre_text("volatile ");
    RegisterBuiltin("volatile", lhs_tycon_builtin);
    lhs_tycon->set_pre_text("restrict ");
    RegisterBuiltin("restrict", lhs_tycon_builtin);
  }

  {
    MarkedSource rhs_tycon_builtin;
    auto* lookup = rhs_tycon_builtin.add_child();
    auto* rhs_tycon = rhs_tycon_builtin.add_child();
    lookup->set_kind(MarkedSource::LOOKUP_BY_PARAM);
    lookup->set_lookup_index(1);
    rhs_tycon->set_kind(MarkedSource::IDENTIFIER);
    rhs_tycon->set_pre_text("*");
    RegisterBuiltin("ptr", rhs_tycon_builtin);
    rhs_tycon->set_pre_text("&");
    RegisterBuiltin("lvr", rhs_tycon_builtin);
    rhs_tycon->set_pre_text("&&");
    RegisterBuiltin("rvr", rhs_tycon_builtin);
    rhs_tycon->set_pre_text("[incomplete]");
    RegisterBuiltin("iarr", rhs_tycon_builtin);
    rhs_tycon->set_pre_text("[const]");
    RegisterBuiltin("carr", rhs_tycon_builtin);
    rhs_tycon->set_pre_text("[dependent]");
    RegisterBuiltin("darr", rhs_tycon_builtin);
  }

  {
    MarkedSource mem_ptr_tycon_builtin;
    auto* pointee_type = mem_ptr_tycon_builtin.add_child();
    pointee_type->set_kind(MarkedSource::LOOKUP_BY_PARAM);
    pointee_type->set_lookup_index(1);
    auto* class_type = mem_ptr_tycon_builtin.add_child();
    class_type->set_kind(MarkedSource::LOOKUP_BY_PARAM);
    class_type->set_lookup_index(2);
    class_type->set_pre_text(" ");
    class_type->set_post_text("::");
    auto* ident = mem_ptr_tycon_builtin.add_child();
    ident->set_kind(MarkedSource::IDENTIFIER);
    ident->set_pre_text("*");
    RegisterBuiltin("mptr", mem_ptr_tycon_builtin);
  }

  {
    MarkedSource function_tycon_builtin;
    auto* return_type = function_tycon_builtin.add_child();
    return_type->set_kind(MarkedSource::LOOKUP_BY_PARAM);
    return_type->set_lookup_index(1);
    auto* args = function_tycon_builtin.add_child();
    args->set_kind(MarkedSource::PARAMETER_LOOKUP_BY_PARAM);
    args->set_pre_text("(");
    args->set_post_child_text(", ");
    args->set_post_text(")");
    args->set_lookup_index(2);
    RegisterBuiltin("fn", function_tycon_builtin);
    auto* vararg_keyword = function_tycon_builtin.add_child();
    vararg_keyword->set_kind(MarkedSource::IDENTIFIER);
    vararg_keyword->set_pre_text("vararg");
    RegisterBuiltin("fnvararg", function_tycon_builtin);
  }
}

void KytheGraphObserver::EmitBuiltin(Builtin* builtin) const {
  // TODO(shahms): We should not probably not emit anything from const member
  // functions and this is called from them.
  builtin->emitted = true;
  VNameRef ref(VNameRefFromNodeId(builtin->node_id));
  recorder_->AddProperty(ref, NodeKindID::kTBuiltin);
  recorder_->AddMarkedSource(ref, builtin->marked_source);
}

void KytheGraphObserver::EmitMetaNodes() {
  auto EmitNode = [&](const std::string& name,
                      const MarkedSource& marked_source) {
    auto id =
        NodeId::CreateUncompressed(getDefaultClaimToken(), name + "#meta");
    VNameRef ref(VNameRefFromNodeId(id));
    recorder_->AddProperty(ref, NodeKindID::kMeta);
    recorder_->AddMarkedSource(ref, marked_source);
  };
  MarkedSource tapp_signature;
  auto* ctor_lookup = tapp_signature.add_child();
  ctor_lookup->set_kind(MarkedSource::LOOKUP_BY_PARAM);
  ctor_lookup->set_lookup_index(0);
  auto* tapp_body = tapp_signature.add_child();
  tapp_body->set_kind(MarkedSource::PARAMETER_LOOKUP_BY_PARAM_WITH_DEFAULTS);
  tapp_body->set_pre_text("<");
  tapp_body->set_lookup_index(1);
  tapp_body->set_post_child_text(", ");
  tapp_body->set_post_text(">");
  EmitNode("tapp", tapp_signature);
}
}  // namespace kythe
