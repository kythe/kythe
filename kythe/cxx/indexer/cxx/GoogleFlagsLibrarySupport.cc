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

// This file uses the Clang style conventions.

#include "GoogleFlagsLibrarySupport.h"

#include "IndexerASTHooks.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/FileManager.h"

namespace kythe {

/// \return true if `SpellingLoc` exists in the scratch space, is in a macro,
/// or is invalid.
static bool SpellingLocIsImaginary(const clang::SourceManager& SM,
                                   clang::SourceLocation SpellingLoc) {
  if (!SpellingLoc.isValid() || !SpellingLoc.isFileID()) {
    return true;
  }
  auto PLoc = SM.getPresumedLoc(SpellingLoc);
  // There doesn't appear to be a way to get at Preprocessor's ScratchBuf,
  // but we do know that it always allocates files with this special name.
  return !(PLoc.isInvalid() || strcmp(PLoc.getFilename(), "<scratch space>"));
}

/// \brief If `Decl` is a googleflags flag, returns the range covering the flag
/// name in the DECLARE_ or DEFINE_ macro that declared it; otherwise returns an
/// invalid range.
///
/// This function is used in two contexts: first, if you have a variable
/// declaration statement, it will tell you whether that declaration came from a
/// googleflags macro. Second, if you have a reference to a variable at some
/// location in the source text (e.g., a DeclRefExpr), it will tell you
/// whether the VarDecl came from a googleflags macro _and_ whether the location
/// at which the reference expression was made was internal to googleflags.
/// These internal references are googleflags implementation details and are
/// uninteresting to index as high-level references to flag nodes.
///
/// \param LO The LangOptions used to check the input AST.
/// \param Decl The VarDecl that may belong to a flag.
/// \param RefLoc If valid, the location of the reference made to Decl.
static clang::SourceRange GetVarDeclFlagDeclLoc(
    const clang::LangOptions& LO, const clang::VarDecl* Decl,
    clang::SourceLocation RefLoc = clang::SourceLocation()) {
  // Quickly bail out if this isn't "FLAGS_foo":
  if (!Decl->getName().startswith("FLAGS_")) {
    return clang::SourceLocation();
  }
  clang::SourceLocation Loc = Decl->getLocation();
  // Look for an identifier that's a token paste (so it lives inside the
  // scratch buffer).
  if (!Loc.isMacroID() || !Loc.isValid()) {
    return clang::SourceLocation();
  }
  const auto& Context = Decl->getASTContext();
  const auto& SM = Context.getSourceManager();
  // Reject references that come from imaginary places (like token pastes in
  // flags headers).
  if (RefLoc.isValid() && RefLoc.isMacroID() &&
      SpellingLocIsImaginary(SM, SM.getSpellingLoc(RefLoc))) {
    return clang::SourceLocation();
  }
  auto SpellingLoc = SM.getSpellingLoc(Loc);
  if (!SpellingLocIsImaginary(SM, SpellingLoc)) {
    return clang::SourceLocation();
  }
  // Now check to see where the token paste came from. As it turns out, the
  // spelling location of the VarDecl's spelling source range will point into
  // the gflags headers:
  auto IsDefinedInGflagsHeader = [&SM](clang::SourceLocation SRBegin) {
    if (!SRBegin.isValid()) {
      return false;
    }
    auto SRSpellingLoc = SM.getSpellingLoc(SRBegin);
    // Did this come from a googleflags header file?
    auto MaybeFlagsFileId = SM.getFileID(SRSpellingLoc);
    if (MaybeFlagsFileId.isInvalid()) {
      return false;
    }
    const auto* MaybeFlagsFileEntry = SM.getFileEntryForID(MaybeFlagsFileId);
    if (!MaybeFlagsFileEntry) {
      return false;
    }
    llvm::StringRef MaybeFlagsFilename(MaybeFlagsFileEntry->getName());
    return MaybeFlagsFilename.endswith("gflags.h") ||
           MaybeFlagsFilename.endswith("gflags_declare.h");
  };
  auto SRBegin = Decl->getSourceRange().getBegin();
  if (!SRBegin.isValid() ||
      (!IsDefinedInGflagsHeader(SRBegin) &&
       !IsDefinedInGflagsHeader(
           SM.getImmediateExpansionRange(SRBegin).getBegin()))) {
    return clang::SourceLocation();
  }
  // This VarDecl's name (which starts with FLAGS_) came from a token paste
  // that was the result of a macro that was written down in a googleflags
  // include file. The last thing we'll check is that the macro that we
  // originally expanded was one of the DEFINE_ or DECLARE_ gflags macros.
  bool WasInvalid = false;
  auto FileLoc = SM.getFileLoc(Loc);
  auto FileIdOffset = SM.getDecomposedLoc(FileLoc);
  auto FileBuf = SM.getBufferData(FileIdOffset.first, &WasInvalid);
  if (WasInvalid) {
    return clang::SourceLocation();
  }
  auto DefTokenEnd = clang::Lexer::getLocForEndOfToken(FileLoc, 0, SM, LO);
  if (DefTokenEnd.isInvalid()) {
    return clang::SourceLocation();
  }
  auto FileIdEndOffset = SM.getDecomposedLoc(DefTokenEnd);
  if (FileIdEndOffset.first != FileIdOffset.first) {
    // These aren't in the same file somehow?
    return clang::SourceLocation();
  }
  auto DefTokText = llvm::StringRef(FileBuf.substr(
      FileIdOffset.second, FileIdEndOffset.second - FileIdOffset.second));
  bool IsFlagDef =
      DefTokText.startswith("DEFINE_") || DefTokText.startswith("ABSL_FLAG");
  if (!IsFlagDef && !DefTokText.startswith("DECLARE_")) {
    return clang::SourceLocation();
  }
  if (IsFlagDef && (Decl->getDefinition() != Decl)) {
    // Reject internal declarations.
    return clang::SourceLocation();
  }
  // Note that we still get FLAGS_foo, FLAGS_nofoo, FLAGS_nonofoo. We'll lex the
  // `(` and `id` from `DEFINE_ttt(id` and compare with the identifier name.
  // Only when they match will we return successfully.
  clang::Token MaybeParen;
  bool LexBad = clang::Lexer::getRawToken(DefTokenEnd, MaybeParen, SM, LO,
                                          /* IgnoreWhitespace */ true);
  if (LexBad || !MaybeParen.is(clang::tok::l_paren)) {
    return clang::SourceLocation();
  }
  clang::Token MaybeFlagId;
  LexBad = clang::Lexer::getRawToken(MaybeParen.getEndLoc(), MaybeFlagId, SM,
                                     LO, /* IgnoreWhitespace */ true);
  if (LexBad || !MaybeFlagId.is(clang::tok::raw_identifier)) {
    return clang::SourceLocation();
  }
  auto MaybeFlagName = MaybeFlagId.getRawIdentifier();
  if (MaybeFlagName == Decl->getName().drop_front(6 /* FLAGS_ */)) {
    // Use this negative offset in case we skipped whitespace before the
    // raw_identifier token.
    return clang::SourceRange(
        MaybeFlagId.getEndLoc().getLocWithOffset(-MaybeFlagName.size()),
        MaybeFlagId.getEndLoc());
  }
  return clang::SourceLocation();
}

/// \brief Given the NodeId of the primary variable defn or decl of a flag,
/// returns a NodeId for the flag itself.
/// \param VarId the NodeId for the flag's primary variable (not a _no or _nono)
static GraphObserver::NodeId NodeIdForFlag(GraphObserver& Observer,
                                           const GraphObserver::NodeId& VarId) {
  return Observer.MakeNodeId(VarId.getToken(),
                             "google/gflag#" + VarId.getRawIdentity());
}

void GoogleFlagsLibrarySupport::InspectVariable(
    IndexerASTVisitor& V, const GraphObserver::NodeId& NodeId,
    const clang::VarDecl* Decl, GraphObserver::Completeness Compl,
    const std::vector<Completion>& Compls) {
  GraphObserver& GO = V.getGraphObserver();
  auto Range = GetVarDeclFlagDeclLoc(*GO.getLangOptions(), Decl);
  if (Range.isValid()) {
    auto FlagName = clang::Lexer::getSourceText(
        clang::Lexer::getAsCharRange(Range,
                                     Decl->getASTContext().getSourceManager(),
                                     *GO.getLangOptions()),
        Decl->getASTContext().getSourceManager(), *GO.getLangOptions());
    GraphObserver::NameId FlagNameId;
    // NB: Flags are always global.
    FlagNameId.Path = FlagName.str();
    FlagNameId.EqClass = GraphObserver::NameId::NameEqClass::None;
    GraphObserver::NodeId FlagNodeId = NodeIdForFlag(GO, NodeId);
    GO.recordUserDefinedNode(FlagNodeId, "google/gflag", Compl);
    if (auto RCC = V.ExplicitRangeInCurrentContext(Range)) {
      GO.recordDefinitionBindingRange(*RCC, FlagNodeId);
      clang::FileID DeclFile =
          GO.getSourceManager()->getFileID(Range.getBegin());
      // If there are any Completions, this must be a definition.
      for (const auto& C : Compls) {
        if (const auto* NextDecl = llvm::dyn_cast<clang::VarDecl>(C.Decl)) {
          auto NextDeclRange =
              GetVarDeclFlagDeclLoc(*GO.getLangOptions(), NextDecl);
          if (NextDeclRange.isValid()) {
            clang::FileID NextDeclFile =
                GO.getSourceManager()->getFileID(NextDeclRange.getBegin());
            GO.recordCompletion(NodeIdForFlag(GO, C.DeclId), FlagNodeId);
          }
        }
      }
    }
  }
}

void GoogleFlagsLibrarySupport::InspectDeclRef(
    IndexerASTVisitor& V, clang::SourceLocation DeclRefLocation,
    const GraphObserver::Range& Ref, const GraphObserver::NodeId& RefId,
    const clang::NamedDecl* TargetDecl) {
  GraphObserver& GO = V.getGraphObserver();
  const auto* VD = llvm::dyn_cast<const clang::VarDecl>(TargetDecl);
  if (!VD) {
    // We only care about VarDecls.
    return;
  }
  auto Range = GetVarDeclFlagDeclLoc(*GO.getLangOptions(), VD, DeclRefLocation);
  if (Range.isValid()) {
    GO.recordDeclUseLocation(Ref, NodeIdForFlag(GO, RefId),
                             GraphObserver::Claimability::Unclaimable,
                             V.IsImplicit(Ref));
  }
}
}  // namespace kythe
