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

// This file uses the Clang style conventions.

#include "IndexerASTHooks.h"

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
#include "clang/Lex/Lexer.h"

#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

namespace kythe {

using namespace clang;

void *NullGraphObserver::NullClaimToken::NullClaimTokenClass = nullptr;

/// \brief Restores the type of a stacklike container of `ElementType` upon
/// destruction.
template <typename StackType> class StackSizeRestorer {
public:
  explicit StackSizeRestorer(StackType &Target)
      : Target(&Target), Size(Target.size()) {}
  StackSizeRestorer(StackSizeRestorer &O) = delete;
  StackSizeRestorer(StackSizeRestorer &&O) {
    Target = O.Target;
    O.Target = nullptr;
  }
  ~StackSizeRestorer() {
    if (Target) {
      assert(Size <= Target->size());
      while (Size < Target->size()) {
        Target->pop_back();
      }
    }
  }

private:
  StackType *Target;
  decltype(Target->size()) Size;
};

/// \brief Handles the restoration of a stacklike container.
///
/// \example
/// \code
///   auto R = RestoreStack(SomeStack);
/// \endcode
template <typename StackType>
StackSizeRestorer<StackType> RestoreStack(StackType &S) {
  return StackSizeRestorer<StackType>(S);
}

void IndexerASTVisitor::deleteAllParents() {
  if (!AllParents) {
    return;
  }
  for (const auto &Entry : *AllParents) {
    if (Entry.second.is<IndexedParentVector *>()) {
      delete Entry.second.get<IndexedParentVector *>();
    } else {
      delete Entry.second.get<IndexedParent *>();
    }
  }
  AllParents.reset(nullptr);
}

IndexedParentVector IndexerASTVisitor::getIndexedParents(
    const ast_type_traits::DynTypedNode &Node) {
  assert(Node.getMemoizationData() &&
         "Invariant broken: only nodes that support memoization may be "
         "used in the parent map.");
  if (!AllParents) {
    // We always need to run over the whole translation unit, as
    // hasAncestor can escape any subtree.
    // TODO(zarko): Is this relavant for naming?
    AllParents =
        IndexedParentASTVisitor::buildMap(*Context.getTranslationUnitDecl());
  }
  IndexedParentMap::const_iterator I =
      AllParents->find(Node.getMemoizationData());
  if (I == AllParents->end()) {
    return IndexedParentVector();
  }
  if (I->second.is<IndexedParent *>()) {
    return IndexedParentVector(1, *I->second.get<IndexedParent *>());
  }
  const auto &Parents = *I->second.get<IndexedParentVector *>();
  return IndexedParentVector(Parents.begin(), Parents.end());
}

bool IndexerASTVisitor::IsDefinition(const clang::VarDecl *VD) {
  if (const auto *PVD = dyn_cast<ParmVarDecl>(VD)) {
    // For parameters, we want to report them as definitions iff they're
    // part of a function definition.  Current (2013-02-14) Clang appears
    // to report all function parameter declarations as definitions.
    if (const auto *FD = dyn_cast<FunctionDecl>(PVD->getDeclContext())) {
      return IsDefinition(FD);
    }
  }
  // This one's a little quirky.  It would actually work to just return
  // implicit_cast<bool>(VD->isThisDeclarationADefinition()), because
  // VarDecl::DeclarationOnly is zero, but this is more explicit.
  return VD->isThisDeclarationADefinition() != VarDecl::DeclarationOnly;
}

/// \brief Returns true if the `SL` is from a top-level macro argument and
/// the argument itself is not expanded from a macro.
///
/// Example: returns true for `i` and false for `MACRO_INT_VAR` in the
/// following code segment.
///
/// ~~~
/// #define MACRO_INT_VAR i
/// int i;
/// MACRO_FUNCTION(i);              // a top level non-macro macro argument
/// MACRO_FUNCTION(MACRO_INT_VAR);  // a top level _macro_ macro argument
/// ~~~
static bool IsTopLevelNonMacroMacroArgument(const clang::SourceManager &SM,
                                            const clang::SourceLocation &SL) {
  if (!SL.isMacroID())
    return false;
  clang::SourceLocation Loc = SL;
  // We want to get closer towards the initial macro typed into the source only
  // if the location is being expanded as a macro argument.
  while (SM.isMacroArgExpansion(Loc)) {
    // We are calling getImmediateMacroCallerLoc, but note it is essentially
    // equivalent to calling getImmediateSpellingLoc in this context according
    // to Clang implementation. We are not calling getImmediateSpellingLoc
    // because Clang comment says it "should not generally be used by clients."
    Loc = SM.getImmediateMacroCallerLoc(Loc);
  }
  return !Loc.isMacroID();
}

/// \brief Updates `*Loc` to move it past whitespace characters.
///
/// This is useful because most of `clang::Lexer`'s static functions fail if
/// given a location that is in whitespace between tokens.
///
/// TODO(jdennett): Delete this if/when we replace its uses with sane lexing.
void SkipWhitespace(const SourceManager &SM, SourceLocation *Loc) {
  assert(Loc != nullptr);

  while (clang::isWhitespace(*SM.getCharacterData(*Loc))) {
    *Loc = Loc->getLocWithOffset(1);
  }
}

clang::SourceRange IndexerASTVisitor::RangeForOperatorName(
    const clang::SourceRange &OperatorTokenRange) const {
  // Make a longer range than `OperatorTokenRange`, if possible, so that
  // the range captures which operator this is.  There are two kinds of
  // operators; type conversion operators, and overloaded operators.
  // Most of the overloaded operators have names `operator ???` for some
  // token `???`, but there are exceptions (namely: `operator[]`,
  // `operator()`, `operator new[]` and `operator delete[]`.
  //
  // If this is a conversion operator to some type T, we link from the
  // `operator` keyword only, but if it's an overloaded operator then
  // we'd like to link from the name, e.g., `operator[]`, so long as
  // that is spelled out in the source file.
  SourceLocation Pos = OperatorTokenRange.getEnd();
  SkipWhitespace(*Observer.getSourceManager(), &Pos);

  // TODO(jdennett): Find a better name for `Token2EndLocation`, to
  // indicate that it's the end of the first token after `operator`.
  SourceLocation Token2EndLocation = clang::Lexer::getLocForEndOfToken(
      Pos, 0, /* offset from end of token */
      *Observer.getSourceManager(), *Observer.getLangOptions());
  if (!Token2EndLocation.isValid()) {
    // Well, this is the best we can do then.
    return OperatorTokenRange;
  }
  // TODO(jdennett): Prefer checking the token's type here also, rather
  // than the spelling.
  llvm::SmallVector<char, 32> Buffer;
  const clang::StringRef Token2Spelling = clang::Lexer::getSpelling(
      Pos, Buffer, *Observer.getSourceManager(), *Observer.getLangOptions());
  // TODO(jdennett): Handle (and test) operator new, operator new[],
  // operator delete, and operator delete[].
  if (!Token2Spelling.empty() &&
      (Token2Spelling == "::" ||
       clang::Lexer::isIdentifierBodyChar(Token2Spelling[0],
                                          *Observer.getLangOptions()))) {
    // The token after `operator` is an identifier or keyword, or the
    // scope resolution operator `::`, so just return the range of
    // `operator` -- this is presumably a type conversion operator, and
    // the type-visiting code will add any appropriate links from the
    // type, so we shouldn't link from it here.
    return OperatorTokenRange;
  }
  if (Token2Spelling == "(" || Token2Spelling == "[") {
    // TODO(jdennett): Do better than this disgusting hack to skip
    // whitespace.  We probably want to actually instantiate a lexer.
    SourceLocation Pos = Token2EndLocation;
    SkipWhitespace(*Observer.getSourceManager(), &Pos);
    clang::Token Token3;
    if (clang::Lexer::getRawToken(Pos, Token3, *Observer.getSourceManager(),
                                  *Observer.getLangOptions())) {
      // That's weird, we've got just part of the operator name -- we saw
      // an opening "(" or "]", but not its closing partner.  The best
      // we can do is to just return the range covering `operator`.
      llvm::errs() << "Failed to lex token after " << Token2Spelling.str();
      return OperatorTokenRange;
    }

    if (Token3.is(clang::tok::r_square) || Token3.is(clang::tok::r_paren)) {
      SourceLocation EndLocation = clang::Lexer::getLocForEndOfToken(
          Token3.getLocation(), 0, /* offset from end of token */
          *Observer.getSourceManager(), *Observer.getLangOptions());
      return clang::SourceRange(OperatorTokenRange.getBegin(), EndLocation);
    }
    return OperatorTokenRange;
  }

  // In this case we assume we have `operator???`, for some single-token
  // operator name `???`.
  return clang::SourceRange(OperatorTokenRange.getBegin(), Token2EndLocation);
}

clang::SourceRange IndexerASTVisitor::RangeForSingleTokenFromSourceLocation(
    SourceLocation Start) const {
  assert(Start.isFileID());
  const SourceLocation End = clang::Lexer::getLocForEndOfToken(
      Start, 0, /* offset from end of token */
      *Observer.getSourceManager(), *Observer.getLangOptions());
  return clang::SourceRange(Start, End);
}

SourceLocation
IndexerASTVisitor::ConsumeToken(SourceLocation StartLocation,
                                clang::tok::TokenKind ExpectedKind) const {
  clang::Token Token;
  if (getRawToken(StartLocation, Token) == LexerResult::Success) {
    // We can't use findLocationAfterToken() as that also uses "raw" lexing,
    // which does not respect |lang_opts_.CXXOperatorNames| and will happily
    // give |tok::raw_identifier| for tokens such as "compl" and then decide
    // that "compl" doesn't match tok::tilde.  We want raw lexing so that
    // macro expansion is suppressed, but then we need to manually re-map
    // C++'s "alternative tokens" to the correct token kinds.
    clang::tok::TokenKind ActualKind = Token.getKind();
    if (Token.is(clang::tok::raw_identifier)) {
      llvm::StringRef TokenSpelling(Token.getRawIdentifier());
      const clang::IdentifierInfo &II =
          Observer.getPreprocessor()->getIdentifierTable().get(TokenSpelling);
      ActualKind = II.getTokenID();
    }
    if (ActualKind == ExpectedKind) {
      return clang::Lexer::getLocForEndOfToken(
          Token.getLocation(), 0, /* offset after token */
          *Observer.getSourceManager(), *Observer.getLangOptions());
    }
  }
  return SourceLocation(); // invalid location signals error/mismatch.
}

SourceLocation
IndexerASTVisitor::GetLocForEndOfToken(SourceLocation StartLocation) const {
  return clang::Lexer::getLocForEndOfToken(
      StartLocation, 0 /* offset from end of token */,
      *Observer.getSourceManager(), *Observer.getLangOptions());
}

clang::SourceRange IndexerASTVisitor::RangeForNameOfDeclaration(
    const clang::NamedDecl *Decl) const {
  const SourceLocation StartLocation = Decl->getLocation();
  if (StartLocation.isInvalid()) {
    return SourceRange();
  }
  auto dtor = dyn_cast<clang::CXXDestructorDecl>(Decl);
  if (StartLocation.isFileID() && dtor != nullptr) {
    // If the first token is "~" (or its alternate spelling, "compl") and
    // the second is the name of class (rather than the name of a macro),
    // then span two tokens.  Otherwise span just one.
    const SourceLocation NextLocation =
        ConsumeToken(StartLocation, clang::tok::tilde);

    if (NextLocation.isValid()) {
      // There was a tilde (or its alternate token, "compl", which is
      // technically valid for a destructor even if it's awful style).
      // The "name" of the destructor is "~Type" even if the source
      // code says "compl Type".
      clang::Token SecondToken;
      if (getRawToken(NextLocation, SecondToken) == LexerResult::Success &&
          SecondToken.is(clang::tok::raw_identifier) &&
          ("~" + std::string(SecondToken.getRawIdentifier())) ==
              Decl->getNameAsString()) {
        const SourceLocation EndLocation = clang::Lexer::getLocForEndOfToken(
            SecondToken.getLocation(), 0, /* offset from end of token */
            *Observer.getSourceManager(), *Observer.getLangOptions());
        return clang::SourceRange(StartLocation, EndLocation);
      }
    }
  }
  return RangeForASTEntityFromSourceLocation(StartLocation);
}

clang::SourceRange IndexerASTVisitor::RangeForASTEntityFromSourceLocation(
    SourceLocation Start) const {
  if (Start.isFileID()) {
    clang::SourceRange FirstTokenRange =
        RangeForSingleTokenFromSourceLocation(Start);
    llvm::SmallVector<char, 32> Buffer;
    // TODO(jdennett): Prefer to lex a token and then check if it is
    // `clang::tok::kw_operator`, instead of checking the spelling?
    // If we do that, update `RangeForOperatorName` to take a `clang::Token`
    // as its argument?
    llvm::StringRef TokenSpelling =
        clang::Lexer::getSpelling(Start, Buffer, *Observer.getSourceManager(),
                                  *Observer.getLangOptions());
    if (TokenSpelling == "operator") {
      return RangeForOperatorName(FirstTokenRange);
    } else {
      return FirstTokenRange;
    }
  } else {
    // The location is from macro expansion. We always need to return a
    // location that can be associated with the original file.
    SourceLocation FileLoc = Observer.getSourceManager()->getFileLoc(Start);
    if (IsTopLevelNonMacroMacroArgument(*Observer.getSourceManager(), Start)) {
      // TODO(jdennett): Test cases such as `MACRO(operator[])`, where the
      // name in the macro argument should ideally not just be linked to a
      // single token.
      return RangeForSingleTokenFromSourceLocation(FileLoc);
    } else {
      // The entity is in a macro expansion, and it is not a top-level macro
      // argument that itself is not expanded from a macro. The range
      // is a 0-width span (a point), so no source link will be created.
      return clang::SourceRange(FileLoc);
    }
  }
}

void IndexerASTVisitor::MaybeRecordDefinitionRange(
    const GraphObserver::Range &R, const GraphObserver::NodeId &Id) {
  if (R.PhysicalRange.isValid()) {
    Observer.recordDefinitionRange(R, Id);
  }
}

bool IndexerASTVisitor::TraverseFunctionDecl(clang::FunctionDecl *FD) {
  if (unsigned BuiltinID = FD->getBuiltinID()) {
    if (!Observer.getPreprocessor()->getBuiltinInfo().isPredefinedLibFunction(
            BuiltinID)) {
      // These builtins act weirdly (by, e.g., defining themselves inside the
      // macro context where they first appeared, but then also in the global
      // namespace). Don't traverse them.
      return true;
    }
  }
  // Blame calls on actual functions, not on callables. This keeps us from
  // blurring together calls from different functions that happen to alias
  // the same callable.
  auto R = RestoreStack(BlameStack);
  auto S = RestoreStack(RangeContext);
  if (FD->isTemplateInstantiation() &&
      FD->getTemplateSpecializationKind() !=
          clang::TSK_ExplicitSpecialization) {
    // Explicit specializations have ranges.
    RangeContext.push_back(BuildNodeIdForDecl(FD));
  }
  BlameStack.push_back(BuildNodeIdForDecl(FD));
  bool Result =
      clang::RecursiveASTVisitor<IndexerASTVisitor>::TraverseFunctionDecl(FD);
  return Result;
}

bool IndexerASTVisitor::VisitCallExpr(const clang::CallExpr *E) {
  if (const auto *Callee = E->getCalleeDecl()) {
    if (!BlameStack.empty()) {
      clang::SourceLocation RPL = E->getRParenLoc();
      clang::SourceRange SR = E->getSourceRange();
      // This loses the right paren without the offset.
      SR.setEnd(RPL.getLocWithOffset(1));
      GraphObserver::Range RCC = RangeInCurrentContext(SR);
      if (auto CalleeId = BuildNodeIdForCallableDecl(Callee)) {
        Observer.recordCallEdge(RCC, BlameStack.back(), CalleeId.primary());
      }
    }
  }
  return true;
}

MaybeFew<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForDeclContext(const clang::DeclContext *DC) {
  if (auto *DCDecl = llvm::dyn_cast<const clang::Decl>(DC)) {
    if (llvm::isa<TranslationUnitDecl>(DCDecl)) {
      return None();
    }
    if (llvm::isa<ClassTemplatePartialSpecializationDecl>(DCDecl)) {
      return BuildNodeIdForDecl(DCDecl, 0);
    } else if (auto *CRD = dyn_cast<const clang::CXXRecordDecl>(DCDecl)) {
      if (const auto *CTD = CRD->getDescribedClassTemplate()) {
        return BuildNodeIdForDecl(DCDecl, 0);
      }
    } else if (auto *FD = dyn_cast<const clang::FunctionDecl>(DCDecl)) {
      if (FD->getDescribedFunctionTemplate()) {
        return BuildNodeIdForDecl(DCDecl, 0);
      }
    }
    return BuildNodeIdForDecl(DCDecl);
  }
  return None();
}

void IndexerASTVisitor::AddChildOfEdgeToDeclContext(
    const clang::Decl *Decl, const GraphObserver::NodeId DeclNode) {
  if (const DeclContext *DC = Decl->getDeclContext()) {
    if (auto ContextId = BuildNodeIdForDeclContext(DC)) {
      Observer.recordChildOfEdge(DeclNode, ContextId.primary());
    }
  }
}

bool IndexerASTVisitor::VisitDeclRefExpr(const clang::DeclRefExpr *DRE) {
  // Bail on NonTypeTemplateParmDecl for now.
  if (isa<NonTypeTemplateParmDecl>(DRE->getDecl())) {
    return true;
  }
  // TODO(zarko): check to see if this DeclRefExpr has already been indexed.
  // (Use a simple N=1 cache.)
  // Use FoundDecl to get to template defs; use getDecl to get to template
  // instantiations.
  const NamedDecl *const FoundDecl = DRE->getDecl();
  // TODO(zarko): Point at the capture as well as the thing being captured;
  // port over RemapDeclIfCaptured.
  // const NamedDecl* const TargetDecl = RemapDeclIfCaptured(FoundDecl);
  const NamedDecl *const TargetDecl = FoundDecl;
  SourceLocation SL = DRE->getLocation();
  if (SL.isValid()) {
    SourceRange Range = RangeForASTEntityFromSourceLocation(SL);
    GraphObserver::Range RCC = RangeInCurrentContext(Range);
    GraphObserver::NodeId DeclId = BuildNodeIdForDecl(TargetDecl);
    Observer.recordDeclUseLocation(RCC, DeclId,
                                   GraphObserver::Claimability::Unclaimable);
    for (const auto &S : Supports) {
      S->InspectDeclRef(*this, SL, RCC, DeclId, TargetDecl);
    }
  }

  // TODO(zarko): types (if DRE->hasQualifier()...)
  return true;
}

bool IndexerASTVisitor::BuildTemplateArgumentList(
    const clang::ASTTemplateArgumentListInfo *ArgsAsWritten,
    const clang::TemplateArgumentList *Args,
    std::vector<GraphObserver::NodeId> &ArgIds,
    std::vector<const GraphObserver::NodeId *> &ArgNodeIds) {
  ArgIds.clear();
  ArgNodeIds.clear();
  if (ArgsAsWritten) {
    ArgIds.reserve(ArgsAsWritten->NumTemplateArgs);
    ArgNodeIds.reserve(ArgsAsWritten->NumTemplateArgs);
    for (unsigned I = 0; I < ArgsAsWritten->NumTemplateArgs; ++I) {
      if (auto ArgId = BuildNodeIdForTemplateArgument((*ArgsAsWritten)[I],
                                                      EmitRanges::Yes)) {
        ArgIds.push_back(ArgId.primary());
      } else {
        return false;
      }
    }
  } else {
    ArgIds.reserve(Args->size());
    ArgNodeIds.reserve(Args->size());
    for (const auto &Arg : Args->asArray()) {
      if (auto ArgId =
              BuildNodeIdForTemplateArgument(Arg, clang::SourceLocation())) {
        ArgIds.push_back(ArgId.primary());
      } else {
        return false;
      }
    }
  }
  for (const auto &NID : ArgIds) {
    ArgNodeIds.push_back(&NID);
  }
  return true;
}

bool IndexerASTVisitor::VisitVarDecl(const clang::VarDecl *Decl) {
  if (isa<ParmVarDecl>(Decl)) {
    // Ignore parameter types, those are added to the graph after processing
    // the parent function or member.
    return true;
  }
  SourceLocation DeclLoc = Decl->getLocation();
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  GraphObserver::NodeId BodyDeclNode(Observer.getDefaultClaimToken());
  GraphObserver::NodeId DeclNode(Observer.getDefaultClaimToken());
  const clang::ASTTemplateArgumentListInfo *ArgsAsWritten = nullptr;
  if (const auto *VTPSD =
          dyn_cast<const clang::VarTemplatePartialSpecializationDecl>(Decl)) {
    ArgsAsWritten = VTPSD->getTemplateArgsAsWritten();
    BodyDeclNode = BuildNodeIdForDecl(Decl, 0);
    DeclNode = RecordTemplate(VTPSD, BodyDeclNode);
  } else if (const auto *VTD = Decl->getDescribedVarTemplate()) {
    assert(!isa<clang::VarTemplateSpecializationDecl>(VTD));
    BodyDeclNode = BuildNodeIdForDecl(Decl, 0);
    DeclNode = RecordTemplate(VTD, BodyDeclNode);
  } else {
    BodyDeclNode = BuildNodeIdForDecl(Decl);
    DeclNode = BodyDeclNode;
  }

  if (auto *VTSD = dyn_cast<const clang::VarTemplateSpecializationDecl>(Decl)) {
    // If this is a partial specialization, we've already recorded the newly
    // abstracted parameters above. We can now record the type arguments passed
    // to the template we're specializing. Synthesize the type we need.
    std::vector<GraphObserver::NodeId> NIDS;
    std::vector<const GraphObserver::NodeId *> NIDPS;
    auto PrimaryOrPartial = VTSD->getSpecializedTemplateOrPartial();
    if (BuildTemplateArgumentList(ArgsAsWritten, &VTSD->getTemplateArgs(), NIDS,
                                  NIDPS)) {
      if (auto SpecializedNode = BuildNodeIdForTemplateName(
              clang::TemplateName(VTSD->getSpecializedTemplate()),
              VTSD->getPointOfInstantiation())) {
        if (PrimaryOrPartial.is<clang::VarTemplateDecl *>() &&
            !isa<const clang::VarTemplatePartialSpecializationDecl>(Decl)) {
          // This is both an instance and a specialization of the primary
          // template. We use the same arguments list for both.
          Observer.recordInstEdge(
              DeclNode,
              Observer.recordTappNode(SpecializedNode.primary(), NIDPS));
        }
        Observer.recordSpecEdge(
            DeclNode,
            Observer.recordTappNode(SpecializedNode.primary(), NIDPS));
      }
    }
    if (const clang::VarTemplatePartialSpecializationDecl *Partial =
            PrimaryOrPartial
                .dyn_cast<clang::VarTemplatePartialSpecializationDecl *>()) {
      if (BuildTemplateArgumentList(
              nullptr, &VTSD->getTemplateInstantiationArgs(), NIDS, NIDPS)) {
        Observer.recordInstEdge(
            DeclNode,
            Observer.recordTappNode(BuildNodeIdForDecl(Partial), NIDPS));
      }
    }
  }

  MaybeRecordDefinitionRange(RangeInCurrentContext(NameRange), DeclNode);
  GraphObserver::NameId VarNameId(BuildNameIdForDecl(Decl));
  if (const auto *TSI = Decl->getTypeSourceInfo()) {
    // TODO(zarko): Storage classes.
    AscribeSpelledType(TSI->getTypeLoc(), Decl->getType(), BodyDeclNode);
  } else if (auto TyNodeId = BuildNodeIdForType(Decl->getType())) {
    Observer.recordTypeEdge(BodyDeclNode, TyNodeId.primary());
  }
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  std::vector<LibrarySupport::Completion> Completions;
  if (!IsDefinition(Decl)) {
    Observer.recordVariableNode(VarNameId, BodyDeclNode,
                                GraphObserver::Completeness::Incomplete);
    for (const auto &S : Supports) {
      S->InspectVariable(*this, DeclNode, BodyDeclNode, Decl,
                         GraphObserver::Completeness::Incomplete, Completions);
    }
    return true;
  }
  FileID DeclFile = Observer.getSourceManager()->getFileID(Decl->getLocation());
  GraphObserver::Range NameRangeInContext = RangeInCurrentContext(NameRange);
  for (const auto *NextDecl : Decl->redecls()) {
    const clang::Decl *OuterTemplate = nullptr;
    // It's not useful to draw completion edges to implicit forward
    // declarations, nor is it useful to declare that a definition completes
    // itself.
    if (NextDecl != Decl && !NextDecl->isImplicit()) {
      if (auto *VD = dyn_cast<const clang::VarDecl>(NextDecl)) {
        OuterTemplate = VD->getDescribedVarTemplate();
      }
      FileID NextDeclFile =
          Observer.getSourceManager()->getFileID(NextDecl->getLocation());
      // We should not point a completes edge from an abs node to a var node.
      GraphObserver::NodeId TargetDecl =
          BuildNodeIdForDecl(OuterTemplate ? OuterTemplate : NextDecl);
      Observer.recordCompletionRange(
          NameRangeInContext, TargetDecl,
          NextDeclFile == DeclFile
              ? GraphObserver::Specificity::UniquelyCompletes
              : GraphObserver::Specificity::Completes);
      Completions.push_back(LibrarySupport::Completion{NextDecl, TargetDecl});
    }
  }
  Observer.recordVariableNode(VarNameId, BodyDeclNode,
                              GraphObserver::Completeness::Definition);
  for (const auto &S : Supports) {
    S->InspectVariable(*this, DeclNode, BodyDeclNode, Decl,
                       GraphObserver::Completeness::Definition, Completions);
  }
  return true;
}

bool IndexerASTVisitor::VisitEnumConstantDecl(
    const clang::EnumConstantDecl *Decl) {
  // We first build the NameId and NodeId for the enumerator.
  GraphObserver::NameId DeclName(BuildNameIdForDecl(Decl));
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  SourceLocation DeclLoc = Decl->getLocation();
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  MaybeRecordDefinitionRange(RangeInCurrentContext(NameRange), DeclNode);
  Observer.recordNamedEdge(DeclNode, DeclName);
  Observer.recordIntegerConstantNode(DeclNode, Decl->getInitVal());
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  return true;
}

bool IndexerASTVisitor::VisitEnumDecl(const clang::EnumDecl *Decl) {
  GraphObserver::NameId DeclName(BuildNameIdForDecl(Decl));
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  SourceLocation DeclLoc = Decl->getLocation();
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  MaybeRecordDefinitionRange(RangeInCurrentContext(NameRange), DeclNode);
  Observer.recordNamedEdge(DeclNode, DeclName);
  bool HasSpecifiedStorageType = false;
  if (const auto *TSI = Decl->getIntegerTypeSourceInfo()) {
    HasSpecifiedStorageType = true;
    AscribeSpelledType(TSI->getTypeLoc(), TSI->getType(), DeclNode);
  }
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  // TODO(zarko): Would this be clearer as !Decl->isThisDeclarationADefinition
  // or !Decl->isCompleteDefinition()? Do those calls have the same meaning
  // as Decl->getDefinition() != Decl? The Clang documentation suggests that
  // there is a subtle difference.
  // TODO(zarko): Add edges to previous decls.
  if (Decl->getDefinition() != Decl) {
    // TODO(jdennett): Should we use Type::isIncompleteType() instead of doing
    // something enum-specific here?
    Observer.recordEnumNode(
        DeclNode,
        HasSpecifiedStorageType ? GraphObserver::Completeness::Complete
                                : GraphObserver::Completeness::Incomplete,
        Decl->isScoped() ? GraphObserver::EnumKind::Scoped
                         : GraphObserver::EnumKind::Unscoped);
    return true;
  }
  FileID DeclFile = Observer.getSourceManager()->getFileID(Decl->getLocation());
  for (const auto *NextDecl : Decl->redecls()) {
    if (NextDecl != Decl) {
      FileID NextDeclFile =
          Observer.getSourceManager()->getFileID(NextDecl->getLocation());
      Observer.recordCompletionRange(
          RangeInCurrentContext(NameRange), BuildNodeIdForDecl(NextDecl),
          NextDeclFile == DeclFile
              ? GraphObserver::Specificity::UniquelyCompletes
              : GraphObserver::Specificity::Completes);
    }
  }
  Observer.recordEnumNode(DeclNode, GraphObserver::Completeness::Definition,
                          Decl->isScoped() ? GraphObserver::EnumKind::Scoped
                                           : GraphObserver::EnumKind::Unscoped);
  return true;
}

// TODO(zarko): In general, while we traverse a specialization we don't
// want to have the primary-template's type variables in context.
bool IndexerASTVisitor::TraverseClassTemplateDecl(
    clang::ClassTemplateDecl *TD) {
  TypeContext.push_back(TD->getTemplateParameters());
  bool Result =
      RecursiveASTVisitor<IndexerASTVisitor>::TraverseClassTemplateDecl(TD);
  TypeContext.pop_back();
  return Result;
}

// NB: The Traverse* member that's called is based on the dynamic type of the
// AST node it's being called with (so only one of
// TraverseClassTemplate{Partial}SpecializationDecl will be called).
bool IndexerASTVisitor::TraverseClassTemplateSpecializationDecl(
    clang::ClassTemplateSpecializationDecl *TD) {
  auto R = RestoreStack(RangeContext);
  // If this specialization was spelled out in the file, it has
  // physical ranges.
  if (TD->getTemplateSpecializationKind() !=
      clang::TSK_ExplicitSpecialization) {
    RangeContext.push_back(BuildNodeIdForDecl(TD));
  }
  bool Result = RecursiveASTVisitor<
      IndexerASTVisitor>::TraverseClassTemplateSpecializationDecl(TD);
  return Result;
}

bool IndexerASTVisitor::TraverseVarTemplateSpecializationDecl(
    clang::VarTemplateSpecializationDecl *TD) {
  if (TD->getTemplateSpecializationKind() == TSK_Undeclared ||
      TD->getTemplateSpecializationKind() == TSK_ImplicitInstantiation) {
    // We should have hit implicit decls in the primary template.
    return true;
  } else {
    return ForceTraverseVarTemplateSpecializationDecl(TD);
  }
}

bool IndexerASTVisitor::ForceTraverseVarTemplateSpecializationDecl(
    clang::VarTemplateSpecializationDecl *TD) {
  auto R = RestoreStack(RangeContext);
  // If this specialization was spelled out in the file, it has
  // physical ranges.
  if (TD->getTemplateSpecializationKind() !=
      clang::TSK_ExplicitSpecialization) {
    RangeContext.push_back(BuildNodeIdForDecl(TD));
  }
  bool Result = RecursiveASTVisitor<
      IndexerASTVisitor>::TraverseVarTemplateSpecializationDecl(TD);
  return Result;
}

bool IndexerASTVisitor::TraverseClassTemplatePartialSpecializationDecl(
    clang::ClassTemplatePartialSpecializationDecl *TD) {
  // Implicit partial specializations don't happen, so we don't need
  // to consider changing the RangeContext stack.
  TypeContext.push_back(TD->getTemplateParameters());
  bool Result = RecursiveASTVisitor<
      IndexerASTVisitor>::TraverseClassTemplatePartialSpecializationDecl(TD);
  TypeContext.pop_back();
  return Result;
}

bool IndexerASTVisitor::TraverseVarTemplateDecl(clang::VarTemplateDecl *TD) {
  TypeContext.push_back(TD->getTemplateParameters());
  if (!RecursiveASTVisitor<IndexerASTVisitor>::TraverseDecl(
          TD->getTemplatedDecl())) {
    TypeContext.pop_back();
    return false;
  }
  if (TD == TD->getCanonicalDecl()) {
    for (auto *SD : TD->specializations()) {
      for (auto *RD : SD->redecls()) {
        auto *VD = cast<VarTemplateSpecializationDecl>(RD);
        switch (VD->getSpecializationKind()) {
        // Visit the implicit instantiations with the requested pattern.
        case TSK_Undeclared:
        case TSK_ImplicitInstantiation:
          if (!ForceTraverseVarTemplateSpecializationDecl(VD)) {
            TypeContext.pop_back();
            return false;
          }
          break;

        // We don't need to do anything on an explicit instantiation
        // or explicit specialization because there will be an explicit
        // node for it elsewhere.
        case TSK_ExplicitInstantiationDeclaration:
        case TSK_ExplicitInstantiationDefinition:
        case TSK_ExplicitSpecialization:
          break;
        }
      }
    }
  }
  TypeContext.pop_back();
  return true;
}

bool IndexerASTVisitor::TraverseVarTemplatePartialSpecializationDecl(
    clang::VarTemplatePartialSpecializationDecl *TD) {
  // Implicit partial specializations don't happen, so we don't need
  // to consider changing the RangeContext stack.
  TypeContext.push_back(TD->getTemplateParameters());
  bool Result = RecursiveASTVisitor<
      IndexerASTVisitor>::TraverseVarTemplatePartialSpecializationDecl(TD);
  TypeContext.pop_back();
  return Result;
}

bool IndexerASTVisitor::TraverseFunctionTemplateDecl(
    clang::FunctionTemplateDecl *FTD) {
  TypeContext.push_back(FTD->getTemplateParameters());
  // We traverse the template parameter list when we visit the FunctionDecl.
  RecursiveASTVisitor<IndexerASTVisitor>::TraverseDecl(FTD->getTemplatedDecl());
  TypeContext.pop_back();
  // See also RecursiveAstVisitor<T>::TraverseTemplateInstantiations.
  if (FTD == FTD->getCanonicalDecl()) {
    for (auto *FD : FTD->specializations()) {
      for (auto *RD : FD->redecls()) {
        if (RD->getTemplateSpecializationKind() !=
            clang::TSK_ExplicitSpecialization) {
          RecursiveASTVisitor<IndexerASTVisitor>::TraverseDecl(RD);
        }
      }
    }
  }
  return true;
}

GraphObserver::Range
IndexerASTVisitor::RangeInCurrentContext(const clang::SourceRange &SR) {
  if (!RangeContext.empty()) {
    return GraphObserver::Range(SR, RangeContext.back());
  } else {
    return GraphObserver::Range(SR, Observer.getClaimTokenForRange(SR));
  }
}

template <typename TemplateDeclish>
GraphObserver::NodeId
IndexerASTVisitor::RecordTemplate(const TemplateDeclish *Decl,
                                  const GraphObserver::NodeId &BodyDeclNode) {
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  Observer.recordChildOfEdge(BodyDeclNode, DeclNode);
  Observer.recordAbsNode(DeclNode);
  for (const auto *ND : *Decl->getTemplateParameters()) {
    GraphObserver::NodeId ParamId(Observer.getDefaultClaimToken());
    unsigned ParamIndex = 0;
    if (const auto *TTPD = dyn_cast<clang::TemplateTypeParmDecl>(ND)) {
      ParamId = BuildNodeIdForDecl(ND);
      Observer.recordAbsVarNode(ParamId);
      ParamIndex = TTPD->getIndex();
    } else if (const auto *NTTPD =
                   dyn_cast<clang::NonTypeTemplateParmDecl>(ND)) {
      ParamId = BuildNodeIdForDecl(ND);
      Observer.recordAbsVarNode(ParamId);
      ParamIndex = NTTPD->getIndex();
    } else if (const auto *TTPD =
                   dyn_cast<clang::TemplateTemplateParmDecl>(ND)) {
      // We make the external Abs the primary node for TTPD so that
      // uses of the ParmDecl later on point at the Abs and not the wrapped
      // AbsVar.
      GraphObserver::NodeId ParamBodyId = BuildNodeIdForDecl(ND, 0);
      Observer.recordAbsVarNode(ParamBodyId);
      ParamId = RecordTemplate(TTPD, ParamBodyId);
      ParamIndex = TTPD->getIndex();
    } else {
      assert(0 && "Unknown entry in TemplateParameterList");
    }
    SourceRange Range = RangeForNameOfDeclaration(ND);
    MaybeRecordDefinitionRange(RangeInCurrentContext(Range), ParamId);
    Observer.recordNamedEdge(ParamId, BuildNameIdForDecl(ND));
    Observer.recordParamEdge(DeclNode, ParamIndex, ParamId);
  }
  return DeclNode;
}

bool IndexerASTVisitor::VisitRecordDecl(const clang::RecordDecl *Decl) {
  if (Decl->isInjectedClassName()) {
    // We can't ignore this in ::Traverse* and still make use of the code that
    // traverses template instantiations (since that functionality is marked
    // private), so we have to ignore it here.
    return true;
  }
  if (Decl->isEmbeddedInDeclarator() && Decl->getDefinition() != Decl) {
    return true;
  }

  SourceLocation DeclLoc = Decl->getLocation();
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  GraphObserver::NodeId BodyDeclNode(Observer.getDefaultClaimToken());
  GraphObserver::NodeId DeclNode(Observer.getDefaultClaimToken());
  const clang::ASTTemplateArgumentListInfo *ArgsAsWritten = nullptr;
  if (const auto *CTPSD =
          dyn_cast<const clang::ClassTemplatePartialSpecializationDecl>(Decl)) {
    ArgsAsWritten = CTPSD->getTemplateArgsAsWritten();
    BodyDeclNode = BuildNodeIdForDecl(Decl, 0);
    DeclNode = RecordTemplate(CTPSD, BodyDeclNode);
  } else if (auto *CRD = dyn_cast<const clang::CXXRecordDecl>(Decl)) {
    if (const auto *CTD = CRD->getDescribedClassTemplate()) {
      assert(!isa<clang::ClassTemplateSpecializationDecl>(CRD));
      BodyDeclNode = BuildNodeIdForDecl(Decl, 0);
      DeclNode = RecordTemplate(CTD, BodyDeclNode);
    } else {
      BodyDeclNode = BuildNodeIdForDecl(Decl);
      DeclNode = BodyDeclNode;
    }
  } else {
    BodyDeclNode = BuildNodeIdForDecl(Decl);
    DeclNode = BodyDeclNode;
  }

  if (auto *CTSD =
          dyn_cast<const clang::ClassTemplateSpecializationDecl>(Decl)) {
    // If this is a partial specialization, we've already recorded the newly
    // abstracted parameters above. We can now record the type arguments passed
    // to the template we're specializing. Synthesize the type we need.
    TemplateName TN(CTSD->getSpecializedTemplate());
    TypeSourceInfo *TSI;
    if (ArgsAsWritten) {
      TemplateArgumentListInfo TALI;
      ArgsAsWritten->copyInto(TALI);
      TSI = Context.getTemplateSpecializationTypeInfo(TN, DeclLoc, TALI);
    } else {
      assert(CTSD->getTypeForDecl() != nullptr);
      QualType QT(CTSD->getTypeForDecl(), 0);
      TSI = Context.getTrivialTypeSourceInfo(QT, DeclLoc);
    }
    if (auto SpecializedType =
            BuildNodeIdForType(TSI->getTypeLoc(), EmitRanges::No)) {
      Observer.recordSpecEdge(DeclNode, SpecializedType.primary());
    }
  }
  MaybeRecordDefinitionRange(RangeInCurrentContext(NameRange), DeclNode);
  Observer.recordNamedEdge(DeclNode, BuildNameIdForDecl(Decl));
  GraphObserver::RecordKind RK =
      (Decl->isClass() ? GraphObserver::RecordKind::Class
                       : (Decl->isStruct() ? GraphObserver::RecordKind::Struct
                                           : GraphObserver::RecordKind::Union));
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  // TODO(zarko): Would this be clearer as !Decl->isThisDeclarationADefinition
  // or !Decl->isCompleteDefinition()? Do those calls have the same meaning
  // as Decl->getDefinition() != Decl? The Clang documentation suggests that
  // there is a subtle difference.
  // TODO(zarko): Add edges to previous decls.
  if (Decl->getDefinition() != Decl) {
    Observer.recordRecordNode(BodyDeclNode, RK,
                              GraphObserver::Completeness::Incomplete);
    return true;
  }
  FileID DeclFile = Observer.getSourceManager()->getFileID(Decl->getLocation());
  GraphObserver::Range NameRangeInContext = RangeInCurrentContext(NameRange);
  for (const auto *NextDecl : Decl->redecls()) {
    const clang::Decl *OuterTemplate = nullptr;
    // It's not useful to draw completion edges to implicit forward
    // declarations, nor is it useful to declare that a definition completes
    // itself.
    if (NextDecl != Decl && !NextDecl->isImplicit()) {
      if (auto *CRD = dyn_cast<const clang::CXXRecordDecl>(NextDecl)) {
        OuterTemplate = CRD->getDescribedClassTemplate();
      }
      FileID NextDeclFile =
          Observer.getSourceManager()->getFileID(NextDecl->getLocation());
      // We should not point a completes edge from an abs node to a record node.
      GraphObserver::NodeId TargetDecl =
          BuildNodeIdForDecl(OuterTemplate ? OuterTemplate : NextDecl);
      Observer.recordCompletionRange(
          NameRangeInContext, TargetDecl,
          NextDeclFile == DeclFile
              ? GraphObserver::Specificity::UniquelyCompletes
              : GraphObserver::Specificity::Completes);
    }
  }
  if (auto *CRD = dyn_cast<const clang::CXXRecordDecl>(Decl)) {
    for (const auto &BCS : CRD->bases()) {
      if (auto BCSType = BuildNodeIdForType(
              BCS.getTypeSourceInfo()->getTypeLoc(), EmitRanges::Yes)) {
        Observer.recordExtendsEdge(BodyDeclNode, BCSType.primary(),
                                   BCS.isVirtual(), BCS.getAccessSpecifier());
      }
    }
  }
  Observer.recordRecordNode(BodyDeclNode, RK,
                            GraphObserver::Completeness::Definition);
  return true;
}

MaybeFew<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForCallableType(const clang::FunctionDecl *Decl) {
  // Presently, there isn't anything about the normal type of a function that
  // would preclude its use as the type of the callable. If we add things like
  // linkage or calling convention to the function type, this may change.
  const auto *FT = Decl->getFunctionType();
  assert(FT != nullptr);
  return BuildNodeIdForType(QualType(FT, 0));
}

bool IndexerASTVisitor::VisitFunctionDecl(clang::FunctionDecl *Decl) {
  GraphObserver::NodeId InnerNode(Observer.getDefaultClaimToken());
  GraphObserver::NodeId OuterNode(Observer.getDefaultClaimToken());
  // There are five flavors of function (see TemplateOrSpecialization in
  // FunctionDecl).
  const clang::ASTTemplateArgumentListInfo *ArgsAsWritten = nullptr;
  const clang::TemplateArgumentList *Args = nullptr;
  clang::TemplateName TN;
  clang::SourceLocation TNLoc;
  if (auto *FTD = Decl->getDescribedFunctionTemplate()) {
    // Function template (inc. overloads)
    InnerNode = BuildNodeIdForDecl(Decl, 0);
    OuterNode = RecordTemplate(FTD, InnerNode);
  } else if (auto *MSI = Decl->getMemberSpecializationInfo()) {
    // TODO(zarko): This case. Update BuildNodeIdForDeclContext.
    InnerNode = BuildNodeIdForDecl(Decl);
    OuterNode = InnerNode;
  } else if (auto *FTSI = Decl->getTemplateSpecializationInfo()) {
    ArgsAsWritten = FTSI->TemplateArgumentsAsWritten;
    Args = FTSI->TemplateArguments;
    TN = TemplateName(FTSI->getTemplate());
    TNLoc = FTSI->getPointOfInstantiation();
    InnerNode = BuildNodeIdForDecl(Decl);
    OuterNode = InnerNode;
  } else if (auto *DFTSI = Decl->getDependentSpecializationInfo()) {
    // TODO(zarko): This case. Update BuildNodeIdForDeclContext.
    InnerNode = BuildNodeIdForDecl(Decl);
    OuterNode = InnerNode;
  } else {
    // Nothing to do with templates.
    InnerNode = BuildNodeIdForDecl(Decl);
    OuterNode = InnerNode;
  }

  if (ArgsAsWritten || Args) {
    bool CouldGetAllTypes = true;
    std::vector<GraphObserver::NodeId> NIDS;
    std::vector<const GraphObserver::NodeId *> NIDPS;
    if (ArgsAsWritten) {
      NIDS.reserve(ArgsAsWritten->NumTemplateArgs);
      NIDPS.reserve(ArgsAsWritten->NumTemplateArgs);
      for (unsigned I = 0; I < ArgsAsWritten->NumTemplateArgs; ++I) {
        if (auto ArgId = BuildNodeIdForTemplateArgument((*ArgsAsWritten)[I],
                                                        EmitRanges::Yes)) {
          NIDS.push_back(ArgId.primary());
        } else {
          CouldGetAllTypes = false;
          break;
        }
      }
    } else {
      NIDS.reserve(Args->size());
      NIDPS.reserve(Args->size());
      for (unsigned I = 0; I < Args->size(); ++I) {
        if (auto ArgId = BuildNodeIdForTemplateArgument(
                Args->get(I), clang::SourceLocation())) {
          NIDS.push_back(ArgId.primary());
        } else {
          CouldGetAllTypes = false;
          break;
        }
      }
    }
    if (CouldGetAllTypes) {
      if (auto SpecializedNode = BuildNodeIdForTemplateName(TN, TNLoc)) {
        for (const auto &NID : NIDS) {
          NIDPS.push_back(&NID);
        }
        Observer.recordSpecEdge(
            OuterNode,
            Observer.recordTappNode(SpecializedNode.primary(), NIDPS));
      }
    }
  }

  GraphObserver::NameId DeclName(BuildNameIdForDecl(Decl));
  auto CallableDeclNode(BuildNodeIdForCallableDecl(Decl));
  SourceLocation DeclLoc = Decl->getLocation();
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  GraphObserver::Range NameRangeInContext(RangeInCurrentContext(NameRange));
  MaybeRecordDefinitionRange(NameRangeInContext, OuterNode);
  Observer.recordNamedEdge(OuterNode, DeclName);
  if (CallableDeclNode) {
    Observer.recordCallableAsEdge(OuterNode, CallableDeclNode.primary());
  }
  bool IsFunctionDefinition = IsDefinition(Decl);
  unsigned ParamNumber = 0;
  for (const auto *Param : Decl->params()) {
    GraphObserver::NodeId VarNodeId(BuildNodeIdForDecl(Param));
    GraphObserver::NameId VarNameId(BuildNameIdForDecl(Param));
    SourceRange Range = RangeForNameOfDeclaration(Param);
    Observer.recordVariableNode(VarNameId, VarNodeId,
                                IsFunctionDefinition
                                    ? GraphObserver::Completeness::Definition
                                    : GraphObserver::Completeness::Incomplete);
    MaybeRecordDefinitionRange(RangeInCurrentContext(Range), VarNodeId);
    Observer.recordParamEdge(InnerNode, ParamNumber++, VarNodeId);
    MaybeFew<GraphObserver::NodeId> ParamType;
    if (auto *TSI = Param->getTypeSourceInfo()) {
      ParamType = BuildNodeIdForType(TSI->getTypeLoc(), EmitRanges::No);
    } else {
      assert(!Param->getType().isNull());
      ParamType = BuildNodeIdForType(
          Context.getTrivialTypeSourceInfo(Param->getType(), Range.getBegin())
              ->getTypeLoc(),
          EmitRanges::No);
    }
    if (ParamType) {
      Observer.recordTypeEdge(VarNodeId, ParamType.primary());
    }
  }

  MaybeFew<GraphObserver::NodeId> FunctionType;
  if (auto *TSI = Decl->getTypeSourceInfo()) {
    FunctionType =
        BuildNodeIdForType(TSI->getTypeLoc(), Decl->getType(), EmitRanges::Yes);
  } else {
    FunctionType = BuildNodeIdForType(
        Context.getTrivialTypeSourceInfo(Decl->getType(), NameRange.getBegin())
            ->getTypeLoc(),
        EmitRanges::No);
  }

  if (FunctionType) {
    Observer.recordTypeEdge(InnerNode, FunctionType.primary());
  }

  // TODO(zarko): Don't generate the callable node redundantly. (This might
  // be a TODO for other calls to BuildNodeIdForCallableDecl--maybe these can
  // assume that the CallableDeclNode will be generated elsewhere.)
  if (Decl->isFirstDecl()) {
    if (CallableDeclNode) {
      Observer.recordCallableNode(CallableDeclNode.primary());
      if (auto CallableType = BuildNodeIdForCallableType(Decl)) {
        Observer.recordTypeEdge(CallableDeclNode.primary(),
                                CallableType.primary());
      }
    }
    if (const auto *MF = dyn_cast<CXXMethodDecl>(Decl)) {
      const auto *R = MF->getParent();
      GraphObserver::NodeId ParentNode(BuildNodeIdForDecl(R));
      // OO_Call, OO_Subscript, and OO_Equal must be member functions.
      // The dyn_cast to CXXMethodDecl above is therefore not dropping
      // (impossible) free function incarnations of these operators from
      // consideration in the following.
      if (MF->getOverloadedOperator() == clang::OO_Call && CallableDeclNode) {
        Observer.recordCallableAsEdge(ParentNode, CallableDeclNode.primary());
      }
    }
  }
  AddChildOfEdgeToDeclContext(Decl, OuterNode);
  if (!IsFunctionDefinition) {
    Observer.recordFunctionNode(InnerNode,
                                GraphObserver::Completeness::Incomplete);
    return true;
  }
  FileID DeclFile = Observer.getSourceManager()->getFileID(Decl->getLocation());
  for (const auto *NextDecl : Decl->redecls()) {
    const clang::Decl *OuterTemplate = nullptr;
    if (NextDecl != Decl) {
      const clang::Decl *OuterTemplate =
          NextDecl->getDescribedFunctionTemplate();
      FileID NextDeclFile =
          Observer.getSourceManager()->getFileID(NextDecl->getLocation());
      GraphObserver::NodeId TargetDecl =
          BuildNodeIdForDecl(OuterTemplate ? OuterTemplate : NextDecl);
      Observer.recordCompletionRange(
          NameRangeInContext, TargetDecl,
          NextDeclFile == DeclFile
              ? GraphObserver::Specificity::UniquelyCompletes
              : GraphObserver::Specificity::Completes);
    }
  }
  Observer.recordFunctionNode(InnerNode,
                              GraphObserver::Completeness::Definition);
  return true;
}

bool IndexerASTVisitor::VisitTypedefNameDecl(
    const clang::TypedefNameDecl *Decl) {
  if (Decl == Context.getBuiltinVaListDecl() ||
      Decl == Context.getInt128Decl() || Decl == Context.getUInt128Decl()) {
    // Don't index __uint128_t, __builtin_va_list, __int128_t
    return true;
  }
  SourceRange Range = RangeForNameOfDeclaration(Decl);
  clang::TypeSourceInfo *TSI = Decl->getTypeSourceInfo();
  if (auto AliasedTypeId =
          BuildNodeIdForType(TSI->getTypeLoc(), EmitRanges::Yes)) {
    GraphObserver::NameId AliasNameId(BuildNameIdForDecl(Decl));
    GraphObserver::NodeId AliasNodeId(
        Observer.recordTypeAliasNode(AliasNameId, AliasedTypeId.primary()));
    MaybeRecordDefinitionRange(RangeInCurrentContext(Range), AliasNodeId);
    AddChildOfEdgeToDeclContext(Decl, AliasNodeId);
  }
  return true;
}

void IndexerASTVisitor::AscribeSpelledType(
    const clang::TypeLoc &Type, const clang::QualType &TrueType,
    const GraphObserver::NodeId &AscribeTo) {
  if (auto TyNodeId = BuildNodeIdForType(Type, TrueType, EmitRanges::Yes)) {
    Observer.recordTypeEdge(AscribeTo, TyNodeId.primary());
  }
}

GraphObserver::NameId::NameEqClass
IndexerASTVisitor::BuildNameEqClassForDecl(const clang::Decl *D) {
  assert(D != nullptr);
  if (const auto *T = dyn_cast<clang::TagDecl>(D)) {
    switch (T->getTagKind()) {
    case clang::TTK_Struct:
      return GraphObserver::NameId::NameEqClass::Class;
    case clang::TTK_Class:
      return GraphObserver::NameId::NameEqClass::Class;
    case clang::TTK_Union:
      return GraphObserver::NameId::NameEqClass::Union;
    default:
      // TODO(zarko): Add classes for other tag kinds, like enums.
      return GraphObserver::NameId::NameEqClass::None;
    }
  } else if (const auto *T = dyn_cast<clang::ClassTemplateDecl>(D)) {
    // Eqclasses should see through templates.
    return BuildNameEqClassForDecl(T->getTemplatedDecl());
  }
  return GraphObserver::NameId::NameEqClass::None;
}

/// \brief Attempts to add some representation of `ND` to `Ostream`.
/// \return true on success; false on failure.
bool IndexerASTVisitor::AddNameToStream(llvm::raw_string_ostream &Ostream,
                                        const clang::NamedDecl *ND) {
  // NamedDecls without names exist--e.g., unnamed namespaces.
  auto Name = ND->getDeclName();
  auto II = Name.getAsIdentifierInfo();
  if (II && !II->getName().empty()) {
    Ostream << II->getName();
  } else {
    if (isa<NamespaceDecl>(ND)) {
      // Even if it doesn't strictly reflect what the standard says, it's
      // useful to collapse these unnamed namespaces into a single
      // namespace.
      Ostream << "@";
    } else if (Name.getCXXOverloadedOperator() != OO_None) {
      switch (Name.getCXXOverloadedOperator()) {
#define OVERLOADED_OPERATOR(Name, Spelling, Token, Unary, Binary, MemberOnly)  \
  case OO_##Name:                                                              \
    Ostream << "OO#" << #Name;                                                 \
    break;
#include "clang/Basic/OperatorKinds.def"
#undef OVERLOADED_OPERATOR
      default:
        return false;
        break;
      }
    } else if (const auto *RD = dyn_cast<clang::RecordDecl>(ND)) {
      Ostream << HashToString(SemanticHash(RD));
    } else if (const auto *ED = dyn_cast<clang::EnumDecl>(ND)) {
      Ostream << HashToString(SemanticHash(ED));
    } else {
      // Other NamedDecls-sans-names are given parent-dependent names.
      return false;
    }
  }
  return true;
}

GraphObserver::NameId
IndexerASTVisitor::BuildNameIdForDecl(const clang::Decl *Decl) {
  GraphObserver::NameId Id;
  Id.EqClass = BuildNameEqClassForDecl(Decl);
  // Cons onto the end of the name instead of the beginning to optimize for
  // prefix search.
  llvm::raw_string_ostream Ostream(Id.Path);
  bool MissingSeparator = false;
  clang::ast_type_traits::DynTypedNode CurrentNode =
      clang::ast_type_traits::DynTypedNode::create(*Decl);
  const clang::Decl *CurrentNodeAsDecl;
  while (!(CurrentNodeAsDecl = CurrentNode.get<clang::Decl>()) ||
         !isa<clang::TranslationUnitDecl>(CurrentNodeAsDecl)) {
    // TODO(zarko): Do we need to deal with nodes with no memoization data?
    // According to ASTTypeTrates.h:205, only Stmt, Decl, Type and
    // NestedNameSpecifier return memoization data. Can we claim an invariant
    // that if we start at any Decl, we will always encounter nodes with
    // memoization data?
    IndexedParentVector IPV = getIndexedParents(CurrentNode);
    if (IPV.empty()) {
      // Make sure that we don't miss out on implicit nodes.
      if (CurrentNodeAsDecl && CurrentNodeAsDecl->isImplicit()) {
        if (const NamedDecl *ND = dyn_cast<NamedDecl>(CurrentNodeAsDecl)) {
          if (!AddNameToStream(Ostream, ND)) {
            if (const DeclContext *DC = ND->getDeclContext()) {
              if (DC->isFunctionOrMethod()) {
                // Heroically try to come up with a disambiguating identifier,
                // even when the IndexedParentVector is empty. This can happen
                // in anonymous parameter declarations that belong to function
                // prototypes.
                const clang::FunctionDecl *FD =
                    static_cast<const clang::FunctionDecl *>(DC);
                int param_count = 0, found_param = -1;
                for (const auto *P : FD->params()) {
                  if (ND == P) {
                    found_param = param_count;
                    break;
                  }
                  ++param_count;
                }
                Ostream << "@#" << found_param;
              }
            }
            Ostream << "@unknown@";
          }
        }
      }
      break;
    }
    // Pick the first path we took to get to this node.
    IndexedParent IP = IPV[0];
    // We would rather name 'template <etc> class C' as C, not C::C, but
    // we also want to be able to give useful names to templates when they're
    // explicitly requested. Therefore:
    if (MissingSeparator && CurrentNodeAsDecl &&
        isa<ClassTemplateDecl>(CurrentNodeAsDecl)) {
      CurrentNode = IP.Parent;
      continue;
    }
    if (MissingSeparator) {
      Ostream << ":";
    } else {
      MissingSeparator = true;
    }
    if (CurrentNodeAsDecl) {
      // TODO(zarko): check for other specializations and emit accordingly
      // Alternately, maybe it would be better to just always emit the hash?
      // At any rate, a hash cache might be a good idea.
      if (const NamedDecl *ND = dyn_cast<NamedDecl>(CurrentNodeAsDecl)) {
        if (!AddNameToStream(Ostream, ND)) {
          Ostream << IP.Index;
        }
      } else {
        // If there's no good name for this Decl, name it after its child
        // index wrt its parent node.
        Ostream << IP.Index;
      }
    } else if (auto *S = CurrentNode.get<clang::Stmt>()) {
      // This is a Stmt--we can name it by its index wrt its parent node.
      Ostream << IP.Index;
    }
    CurrentNode = IP.Parent;
    if (CurrentNodeAsDecl) {
      if (const auto *DC = CurrentNodeAsDecl->getDeclContext()) {
        if (const TagDecl *TD = dyn_cast<TagDecl>(CurrentNodeAsDecl)) {
          const clang::Decl *DCD;
          switch (TD->getDeclKind()) {
          case Decl::Namespace:
            DCD = cast<NamespaceDecl>(DC);
            break;
          case Decl::TranslationUnit:
            DCD = cast<TranslationUnitDecl>(DC);
            break;
          default:
            DCD = nullptr;
          }
          if (DCD && TD->isEmbeddedInDeclarator()) {
            // Names for declarator-embedded decls should reflect lexical
            // scope, not AST scope.
            CurrentNode = clang::ast_type_traits::DynTypedNode::create(*DCD);
          }
        }
      }
    }
  }
  Ostream.flush();
  return Id;
}

template <typename TemplateDeclish>
uint64_t
IndexerASTVisitor::SemanticHashTemplateDeclish(const TemplateDeclish *Decl) {
  return std::hash<std::string>()(BuildNameIdForDecl(Decl).ToString());
}

uint64_t IndexerASTVisitor::SemanticHash(const clang::TemplateName &TN) {
  switch (TN.getKind()) {
  case TemplateName::Template:
    return SemanticHashTemplateDeclish(TN.getAsTemplateDecl());
  case TemplateName::OverloadedTemplate:
    assert(IgnoreUnimplemented && "SemanticHash(OverloadedTemplate)");
    return 0;
  case TemplateName::QualifiedTemplate:
    assert(IgnoreUnimplemented && "SemanticHash(QualifiedTemplate)");
    return 0;
  case TemplateName::DependentTemplate:
    assert(IgnoreUnimplemented && "SemanticHash(DependentTemplate)");
    return 0;
  case TemplateName::SubstTemplateTemplateParm:
    assert(IgnoreUnimplemented && "SemanticHash(SubstTemplateTemplateParm)");
    return 0;
  case TemplateName::SubstTemplateTemplateParmPack:
    assert(IgnoreUnimplemented &&
           "SemanticHash(SubstTemplateTemplateParmPack)");
    return 0;
  default:
    assert(0 && "Unexpected TemplateName Kind");
  }
  return 0;
}

uint64_t IndexerASTVisitor::SemanticHash(const clang::TemplateArgument &TA) {
  switch (TA.getKind()) {
  case TemplateArgument::Null:
    return 0x1010101001010101LL; // Arbitrary constant for H(Null).
  case TemplateArgument::Type:
    return SemanticHash(TA.getAsType()) ^ 0x2020202002020202LL;
  case TemplateArgument::Declaration:
    assert(IgnoreUnimplemented && "SemanticHash(Declaration)");
    return 0;
  case TemplateArgument::NullPtr:
    assert(IgnoreUnimplemented && "SemanticHash(NullPtr)");
    return 0;
  case TemplateArgument::Integral: {
    auto Val = TA.getAsIntegral();
    if (Val.getMinSignedBits() <= sizeof(uint64_t) * CHAR_BIT) {
      return static_cast<uint64_t>(Val.getExtValue());
    } else {
      return std::hash<std::string>()(Val.toString(10));
    }
  }
  case TemplateArgument::Template:
    return SemanticHash(TA.getAsTemplate()) ^ 0x4040404004040404LL;
  case TemplateArgument::TemplateExpansion:
    assert(IgnoreUnimplemented && "SemanticHash(TemplateExpansion)");
    return 0;
  case TemplateArgument::Expression:
    assert(IgnoreUnimplemented && "SemanticHash(Expression)");
    return 0;
  case TemplateArgument::Pack:
    assert(IgnoreUnimplemented && "SemanticHash(Pack)");
    return 0;
  default:
    assert(0 && "Unexpected TemplateArgument Kind");
  }
  return 0;
}

uint64_t IndexerASTVisitor::SemanticHash(const clang::QualType &T) {
  QualType CQT(T.getCanonicalType());
  return std::hash<std::string>()(CQT.getAsString());
}

uint64_t IndexerASTVisitor::SemanticHash(const clang::EnumDecl *ED) {
  // TODO(zarko): Do we need a better hash function?
  uint64_t hash = 0;
  for (auto E : ED->enumerators()) {
    if (E->getDeclName().isIdentifier()) {
      hash ^= std::hash<std::string>()(E->getName());
    }
  }
  return hash;
}

uint64_t
IndexerASTVisitor::SemanticHash(const clang::TemplateArgumentList *RD) {
  uint64_t hash = 0;
  for (const auto &A : RD->asArray()) {
    hash ^= SemanticHash(A);
  }
  return hash;
}

uint64_t IndexerASTVisitor::SemanticHash(const clang::RecordDecl *RD) {
  // TODO(zarko): Do we need a better hash function? We may need to
  // hash the type variable context all the way up to the root template.
  uint64_t hash = 0;
  for (const auto *D : RD->decls()) {
    if (D->getDeclContext() != RD) {
      // Some decls appear underneath RD in the AST but aren't semantically
      // part of it. For example, in
      //   struct S { struct T *t; };
      // the RecordDecl for T is an AST child of S, but is a DeclContext
      // sibling.
      continue;
    }
    if (const auto *ND = dyn_cast<NamedDecl>(D)) {
      if (ND->getDeclName().isIdentifier()) {
        hash ^= std::hash<std::string>()(ND->getName());
      }
    }
  }
  if (const auto *CR = dyn_cast<CXXRecordDecl>(RD)) {
    if (const auto *TD = CR->getDescribedClassTemplate()) {
      hash ^= SemanticHashTemplateDeclish(TD);
    }
  }
  if (const auto *CTSD =
          dyn_cast<const clang::ClassTemplateSpecializationDecl>(RD)) {
    hash ^= SemanticHash(clang::QualType(CTSD->getTypeForDecl(), 0));
  }
  return hash;
}

MaybeFew<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForCallableDecl(const clang::Decl *Decl) {
  // The callable for a function decl should have the same ID as the callable
  // for the defn of that function. A postprocessing step must determine
  // which defns are available to a given callsite based on linkage information.
  GraphObserver::NameId NameId(BuildNameIdForDecl(Decl));
  GraphObserver::NodeId Id(Observer.getDefaultClaimToken());
  {
    llvm::raw_string_ostream Ostream(Id.Identity);
    Ostream << NameId << "#";
    // Include instantiated type variables.
    clang::ast_type_traits::DynTypedNode CurrentNode =
        clang::ast_type_traits::DynTypedNode::create(*Decl);
    const clang::Decl *CurrentNodeAsDecl;
    while (!(CurrentNodeAsDecl = CurrentNode.get<clang::Decl>()) ||
           !isa<clang::TranslationUnitDecl>(CurrentNodeAsDecl)) {
      IndexedParentVector IPV = getIndexedParents(CurrentNode);
      if (IPV.empty()) {
        break;
      }
      IndexedParent IP = IPV[0];
      CurrentNode = IP.Parent;
      if (!CurrentNodeAsDecl) {
        continue;
      }
      if (const auto *TD = dyn_cast<TemplateDecl>(CurrentNodeAsDecl)) {
        // Disambiguate type abstraction IDs from abstracted type IDs.
        if (CurrentNodeAsDecl != Decl) {
          Ostream << "#";
        }
      } else if (const auto *CTSD = dyn_cast<ClassTemplateSpecializationDecl>(
                     CurrentNodeAsDecl)) {
        Ostream << "#" << HashToString(SemanticHash(
                              &CTSD->getTemplateInstantiationArgs()));
      } else if (const auto *FD = dyn_cast<FunctionDecl>(CurrentNodeAsDecl)) {
        if (const auto *TemplateArgs = FD->getTemplateSpecializationArgs()) {
          Ostream << "#" << HashToString(SemanticHash(TemplateArgs));
        }
      } else if (const auto *VD = dyn_cast<VarTemplateSpecializationDecl>(
                     CurrentNodeAsDecl)) {
        Ostream << "#" << HashToString(SemanticHash(
                              &VD->getTemplateInstantiationArgs()));
      }
    }
    if (const auto *FT = Decl->getFunctionType()) {
      Ostream << HashToString(SemanticHash(QualType(FT, 0)));
    }
    Ostream << "#callable";
  }
  return Id;
}

GraphObserver::NodeId
IndexerASTVisitor::BuildNodeIdForDecl(const clang::Decl *Decl, unsigned Index) {
  GraphObserver::NodeId BaseId(BuildNodeIdForDecl(Decl));
  BaseId.Identity.append("." + std::to_string(Index));
  return BaseId;
}

GraphObserver::NodeId
IndexerASTVisitor::BuildNodeIdForDecl(const clang::Decl *Decl) {
  // We can't assume that no two nodes of the same Kind can appear
  // simultaneously at the same SourceLocation: witness implicit overloaded
  // operator=. We rely on names and types to disambiguate them.
  // NodeIds must be stable across analysis runs with the same input data.
  // Some NodeIds are stable in the face of changes to that data, such as
  // the IDs given to class definitions (in part because of the language rules).

  const auto *Token = Observer.getClaimTokenForLocation(Decl->getLocation());
  GraphObserver::NodeId Id(Token);
  llvm::raw_string_ostream Ostream(Id.Identity);
  Ostream << BuildNameIdForDecl(Decl);

  // First, check to see if this thing is a builtin Decl. These things can
  // pick up weird declaration locations that aren't stable enough for us.
  if (const auto *FD = dyn_cast<FunctionDecl>(Decl)) {
    if (unsigned BuiltinID = FD->getBuiltinID()) {
      Id.Token = Observer.getClaimTokenForBuiltin();
      Ostream << "#builtin";
      return Id;
    }
  }

  // Disambiguate nodes underneath template instances.
  clang::ast_type_traits::DynTypedNode CurrentNode =
      clang::ast_type_traits::DynTypedNode::create(*Decl);
  const clang::Decl *CurrentNodeAsDecl;
  while (!(CurrentNodeAsDecl = CurrentNode.get<clang::Decl>()) ||
         !isa<clang::TranslationUnitDecl>(CurrentNodeAsDecl)) {
    IndexedParentVector IPV = getIndexedParents(CurrentNode);
    if (IPV.empty()) {
      break;
    }
    IndexedParent IP = IPV[0];
    CurrentNode = IP.Parent;
    if (!CurrentNodeAsDecl) {
      continue;
    }
    if (const auto *TD = dyn_cast<TemplateDecl>(CurrentNodeAsDecl)) {
      // Disambiguate type abstraction IDs from abstracted type IDs.
      if (CurrentNodeAsDecl != Decl) {
        Ostream << "#";
      }
    } else if (const auto *CTSD = dyn_cast<ClassTemplateSpecializationDecl>(
                   CurrentNodeAsDecl)) {
      // Inductively, we can break after the first implicit instantiation*
      // (since its NodeId will contain its parent's first implicit
      // instantiation and so on). We still want to include hashes of
      // instantiation types.
      // * we assume that the first parent changing, if it does change, is not
      //   semantically important; we're generating stable internal IDs.
      if (CurrentNodeAsDecl != Decl) {
        Ostream << "#" << BuildNodeIdForDecl(CTSD);
        if (CTSD->isImplicit()) {
          break;
        }
      } else {
        Ostream << "#" << HashToString(SemanticHash(
                              &CTSD->getTemplateInstantiationArgs()));
      }
    } else if (const auto *FD = dyn_cast<FunctionDecl>(CurrentNodeAsDecl)) {
      Ostream << "#"
              << HashToString(SemanticHash(QualType(FD->getFunctionType(), 0)));
      if (const auto *TemplateArgs = FD->getTemplateSpecializationArgs()) {
        if (CurrentNodeAsDecl != Decl) {
          Ostream << "#" << BuildNodeIdForDecl(FD);
          break;
        } else {
          Ostream << "#" << HashToString(SemanticHash(TemplateArgs));
        }
      }
    } else if (const auto *VD =
                   dyn_cast<VarTemplateSpecializationDecl>(CurrentNodeAsDecl)) {
      if (VD->isImplicit()) {
        if (CurrentNodeAsDecl != Decl) {
          Ostream << "#" << BuildNodeIdForDecl(VD);
          break;
        } else {
          Ostream << "#" << HashToString(SemanticHash(
                                &VD->getTemplateInstantiationArgs()));
        }
      }
    }
  }
  // Use hashes to unify otherwise unrelated enums and records across
  // translation units.
  if (const auto *Rec = dyn_cast<clang::RecordDecl>(Decl)) {
    if (Rec->getDefinition() == Rec) {
      Ostream << "#" << HashToString(SemanticHash(Rec));
      return Id;
    }
  } else if (const auto *Enum = dyn_cast<clang::EnumDecl>(Decl)) {
    if (Enum->getDefinition() == Enum) {
      Ostream << "#" << HashToString(SemanticHash(Enum));
      return Id;
    }
  } else if (const auto *ECD = dyn_cast<clang::EnumConstantDecl>(Decl)) {
    if (const auto *E = dyn_cast<clang::EnumDecl>(ECD->getDeclContext())) {
      if (E->getDefinition() == E) {
        Ostream << "#" << HashToString(SemanticHash(E));
        return Id;
      }
    }
  } else if (const auto *FD = dyn_cast<clang::FunctionDecl>(Decl)) {
    if (IsDefinition(FD)) {
      // TODO(zarko): Investigate why Clang colocates incomplete and
      // definition instances of FunctionDecls. This may have been masked
      // for enums and records because of the code above.
      Ostream << "#D";
    }
  } else if (const auto *VD = dyn_cast<clang::VarDecl>(Decl)) {
    if (IsDefinition(VD)) {
      // TODO(zarko): Investigate why Clang colocates incomplete and
      // definition instances of VarDecls. This may have been masked
      // for enums and records because of the code above.
      Ostream << "#D";
    }
  }
  clang::SourceRange DeclRange(Decl->getLocStart(), Decl->getLocEnd());
  auto Range = GraphObserver::Range(DeclRange,
                                    Observer.getClaimTokenForRange(DeclRange));
  Ostream << "@";
  if (!Observer.AppendRangeToStream(Ostream, Range)) {
    Ostream << "invalid";
  }
  return Id;
}

bool IndexerASTVisitor::IsDefinition(const FunctionDecl *FunctionDecl) {
  return FunctionDecl->isThisDeclarationADefinition();
}

// Use the arithmetic sum of the pointer value of clang::Type and the numerical
// value of CVR qualifiers as the unique key for a QualType.
// The size of clang::Type is 24 (as of 2/22/2013), and the maximum of the
// qualifiers (i.e., the return value of clang::Qualifiers::getCVRQualifiers() )
// is clang::Qualifiers::CVRMask which is 7. Therefore, uniqueness is satisfied.
static int64_t ComputeKeyFromQualType(const ASTContext &Context,
                                      const QualType &QT, const Type *T) {
  const clang::SplitQualType &Split = QT.split();
  // split.Ty is of type "const clang::Type*" and uintptr_t is guaranteed
  // to have the same size. Note that reinterpret_cast<int64_t> may fail.
  int64_t Key;
  if (isa<TemplateSpecializationType>(T)) {
    Key = reinterpret_cast<uintptr_t>(Context.getCanonicalType(T));
  } else {
    // Don't collapse aliases if we can help it.
    Key = reinterpret_cast<uintptr_t>(T);
  }
  Key += Split.Quals.getCVRQualifiers();
  return Key;
}

// There aren't too many types in C++, as it turns out. See
// clang/AST/TypeNodes.def.

#define UNSUPPORTED_CLANG_TYPE(t)                                              \
  case TypeLoc::t:                                                             \
    if (IgnoreUnimplemented) {                                                 \
      return None();                                                           \
    } else {                                                                   \
      assert(0 && "TypeLoc::" #t " unsupported");                              \
    }                                                                          \
    break

MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::ApplyBuiltinTypeConstructor(
    const char *BuiltinName, const MaybeFew<GraphObserver::NodeId> &Param) {
  GraphObserver::NodeId TyconID(Observer.getNodeIdForBuiltinType(BuiltinName));
  return Param.Map<GraphObserver::NodeId>(
      [this, &TyconID, &BuiltinName](const GraphObserver::NodeId &Elt) {
        return Observer.recordTappNode(TyconID, {&Elt});
      });
}

MaybeFew<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForTemplateName(const clang::TemplateName &Name,
                                              const clang::SourceLocation L) {
  // TODO(zarko): Do we need to canonicalize `Name`?
  // Maybe with Context.getCanonicalTemplateName()?
  switch (Name.getKind()) {
  case TemplateName::Template: {
    const TemplateDecl *TD = Name.getAsTemplateDecl();
    if (const auto *TTPD = dyn_cast<TemplateTemplateParmDecl>(TD)) {
      return BuildNodeIdForDecl(TTPD);
    } else if (const NamedDecl *UnderlyingDecl = TD->getTemplatedDecl()) {
      if (const auto *TD = dyn_cast<TypeDecl>(UnderlyingDecl)) {
        // TODO(zarko): Under which circumstances is this nullptr?
        // Should we treat this as a type here or as a decl?
        // We've already made the decision elsewhere to link to class
        // definitions directly (in place of nominal nodes), so calling
        // BuildNodeIdForDecl() all the time makes sense. We aren't even
        // emitting ranges.
        const auto *TDType = TD->getTypeForDecl();
        assert(IgnoreUnimplemented || TDType != nullptr);
        if (TDType) {
          QualType QT(TDType, 0);
          TypeSourceInfo *TSI = Context.getTrivialTypeSourceInfo(QT, L);
          return BuildNodeIdForType(TSI->getTypeLoc(), EmitRanges::No);
        } else {
          return None();
        }
      } else if (const auto *FD = dyn_cast<FunctionDecl>(UnderlyingDecl)) {
        // Direct references to function templates to the outer function
        // template shell.
        return Some(BuildNodeIdForDecl(Name.getAsTemplateDecl()));
      } else if (const auto *VD = dyn_cast<VarDecl>(UnderlyingDecl)) {
        // Direct references to variable templates to the appropriate
        // template decl (may be a partial specialization or the
        // primary template).
        return Some(BuildNodeIdForDecl(Name.getAsTemplateDecl()));
      } else {
        assert(0 && "Unexpected UnderlyingDecl");
      }
    } else {
      assert(0 && "BuildNodeIdForTemplateName can't identify TemplateDecl");
    }
  }
  case TemplateName::OverloadedTemplate:
    assert(IgnoreUnimplemented && "TN.OverloadedTemplate");
    break;
  case TemplateName::QualifiedTemplate:
    assert(IgnoreUnimplemented && "TN.QualifiedTemplate");
    break;
  case TemplateName::DependentTemplate:
    assert(IgnoreUnimplemented && "TN.DependentTemplate");
    break;
  case TemplateName::SubstTemplateTemplateParm:
    assert(IgnoreUnimplemented && "TN.SubstTemplateTemplateParmParm");
    break;
  case TemplateName::SubstTemplateTemplateParmPack:
    assert(IgnoreUnimplemented && "TN.SubstTemplateTemplateParmPack");
    break;
  default:
    assert(0 && "Unexpected TemplateName kind!");
  }
  return None();
}

MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::BuildNodeIdForDependentName(
    const clang::NestedNameSpecifierLoc &InNNSLoc,
    const clang::IdentifierInfo *Id, const clang::SourceLocation IdLoc,
    EmitRanges ER) {
  GraphObserver::NodeId IdOut(Observer.getDefaultClaimToken());
  // TODO(zarko): Need a better way to generate stablish names here.
  // In particular, it would be nice if a dependent name A::B::C
  // and a dependent name A::B::D were represented as ::C and ::D off
  // of the same dependent root A::B. (Does this actually make sense,
  // though? Could A::B resolve to a different entity in each case?)
  {
    llvm::raw_string_ostream Ostream(IdOut.Identity);
    Ostream << "#nns"; // Nested name specifier.
    clang::SourceRange NNSRange(InNNSLoc.getBeginLoc(), InNNSLoc.getEndLoc());
    auto Range = RangeInCurrentContext(NNSRange);
    Ostream << "@";
    if (!Observer.AppendRangeToStream(Ostream, Range)) {
      Ostream << "invalid";
    }
  }
  bool HandledRecursively = false;
  unsigned SubIdCount = 0;
  clang::NestedNameSpecifierLoc NNSLoc = InNNSLoc;
  while (NNSLoc && !HandledRecursively) {
    GraphObserver::NodeId SubId(Observer.getDefaultClaimToken());
    auto *NNS = NNSLoc.getNestedNameSpecifier();
    switch (NNS->getKind()) {
    case NestedNameSpecifier::Identifier: {
      // Hashcons the identifiers.
      if (clang::NestedNameSpecifierLoc NNSPrefix = NNSLoc.getPrefix()) {
        if (auto Subtree =
                BuildNodeIdForDependentName(NNSPrefix, NNS->getAsIdentifier(),
                                            NNSLoc.getLocalBeginLoc(), ER)) {
          SubId = Subtree.primary();
          HandledRecursively = true;
        } else {
          assert(IgnoreUnimplemented && "NNS::Identifier");
          return None();
        }
      } else {
        assert(IgnoreUnimplemented && "NNS::Identifier");
        return None();
      }
    } break;
    case NestedNameSpecifier::Namespace:
      assert(IgnoreUnimplemented && "NNS::Namespace");
      return None();
    case NestedNameSpecifier::NamespaceAlias:
      assert(IgnoreUnimplemented && "NNS::NamespaceAlias");
      return None();
    case NestedNameSpecifier::TypeSpec: {
      const TypeLoc &TL = NNSLoc.getTypeLoc();
      if (auto MaybeSubId = BuildNodeIdForType(TL, ER)) {
        SubId = MaybeSubId.primary();
      } else {
        return None();
      }
    } break;
    case NestedNameSpecifier::TypeSpecWithTemplate:
      assert(IgnoreUnimplemented && "NNS::TypeSpecWithTemplate");
      return None();
    case NestedNameSpecifier::Global:
      assert(IgnoreUnimplemented && "NNS::Global");
      return None();
    default:
      assert(IgnoreUnimplemented && "Unexpected NestedNameSpecifier kind.");
      return None();
    }
    Observer.recordParamEdge(IdOut, SubIdCount++, SubId);
    NNSLoc = NNSLoc.getPrefix();
  }
  Observer.recordLookupNode(IdOut, Id->getNameStart());
  if (ER == EmitRanges::Yes) {
    clang::SourceRange Range = RangeForASTEntityFromSourceLocation(IdLoc);
    if (Range.isValid()) {
      Observer.recordDeclUseLocation(RangeInCurrentContext(Range), IdOut);
    }
  }
  return IdOut;
}

MaybeFew<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForExpr(const clang::Expr *Expr, EmitRanges ER) {
  clang::Expr::EvalResult Result;
  GraphObserver::NodeId Id(Observer.getDefaultClaimToken());
  llvm::raw_string_ostream Ostream(Id.Identity);
  if (!Expr->isValueDependent() && Expr->EvaluateAsRValue(Result, Context)) {
    // TODO(zarko): Represent constant values of any type as nodes in the
    // graph; link ranges to them.
    Ostream << Result.Val.getAsString(Context, Expr->getType()) << "#const";
  } else {
    Observer.AppendRangeToStream(
        Ostream, RangeInCurrentContext(
                     RangeForASTEntityFromSourceLocation(Expr->getExprLoc())));
  }
  return Id;
}

// The duplication here is unfortunate, but `TemplateArgumentLoc` is
// different enough from `TemplateArgument * SourceLocation` that
// we can't factor it out.

MaybeFew<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForTemplateArgument(
    const clang::TemplateArgument &Arg, clang::SourceLocation L) {
  // TODO(zarko): Do we need to canonicalize `Arg`?
  // Maybe with Context.getCanonicalTemplateArgument()?
  switch (Arg.getKind()) {
  case TemplateArgument::Null:
    assert(IgnoreUnimplemented && "TA.Null");
    return None();
  case TemplateArgument::Type:
    assert(!Arg.getAsType().isNull());
    return BuildNodeIdForType(
        Context.getTrivialTypeSourceInfo(Arg.getAsType(), L)->getTypeLoc(),
        EmitRanges::No);
  case TemplateArgument::Declaration:
    assert(IgnoreUnimplemented && "TA.Declaration");
    return None();
  case TemplateArgument::NullPtr:
    assert(IgnoreUnimplemented && "TA.NullPtr");
    return None();
  case TemplateArgument::Integral:
    assert(IgnoreUnimplemented && "TA.Integral");
    return None();
  case TemplateArgument::Template:
    return BuildNodeIdForTemplateName(Arg.getAsTemplate(), L);
  case TemplateArgument::TemplateExpansion:
    assert(IgnoreUnimplemented && "TA.TemplateExpansion");
    return None();
  case TemplateArgument::Expression:
    assert(Arg.getAsExpr() != nullptr);
    return BuildNodeIdForExpr(Arg.getAsExpr(), EmitRanges::Yes);
  case TemplateArgument::Pack:
    assert(IgnoreUnimplemented && "TA.Pack");
    return None();
  default:
    assert(IgnoreUnimplemented && "Unexpected TemplateArgument kind!");
  }
  return None();
}

MaybeFew<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForTemplateArgument(
    const clang::TemplateArgumentLoc &ArgLoc, EmitRanges EmitRanges) {
  // TODO(zarko): Do we need to canonicalize `Arg`?
  // Maybe with Context.getCanonicalTemplateArgument()?
  const TemplateArgument &Arg = ArgLoc.getArgument();
  switch (Arg.getKind()) {
  case TemplateArgument::Null:
    assert(IgnoreUnimplemented && "TA.Null");
    return None();
  case TemplateArgument::Type:
    return BuildNodeIdForType(ArgLoc.getTypeSourceInfo()->getTypeLoc(),
                              EmitRanges);
  case TemplateArgument::Declaration:
    assert(IgnoreUnimplemented && "TA.Declaration");
    return None();
  case TemplateArgument::NullPtr:
    assert(IgnoreUnimplemented && "TA.NullPtr");
    return None();
  case TemplateArgument::Integral:
    assert(IgnoreUnimplemented && "TA.Integral");
    return None();
  case TemplateArgument::Template:
    return BuildNodeIdForTemplateName(Arg.getAsTemplate(),
                                      ArgLoc.getTemplateNameLoc());
  case TemplateArgument::TemplateExpansion:
    assert(IgnoreUnimplemented && "TA.TemplateExpansion");
    return None();
  case TemplateArgument::Expression:
    assert(ArgLoc.getSourceExpression() != nullptr);
    return BuildNodeIdForExpr(ArgLoc.getSourceExpression(), EmitRanges);
  case TemplateArgument::Pack:
    assert(IgnoreUnimplemented && "TA.Pack");
    return None();
  default:
    assert(IgnoreUnimplemented && "Unexpected TemplateArgument kind!");
  }
  return None();
}

void IndexerASTVisitor::DumpTypeContext(unsigned Depth, unsigned Index) {
  llvm::errs() << "(looking for " << Depth << "/" << Index << ")\n";
  for (unsigned D = 0; D < TypeContext.size(); ++D) {
    llvm::errs() << "  Depth " << D << " ---- \n";
    for (unsigned I = 0; I < TypeContext[D]->size(); ++I) {
      llvm::errs() << "    Index " << I << " ";
      TypeContext[D]->getParam(I)->dump();
      llvm::errs() << "\n";
    }
  }
}

MaybeFew<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForType(const clang::TypeLoc &Type,
                                      EmitRanges EmitRanges) {
  return BuildNodeIdForType(Type, Type.getTypePtr(), EmitRanges);
}

MaybeFew<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForType(const clang::TypeLoc &Type,
                                      const clang::QualType &QT,
                                      EmitRanges EmitRanges) {
  return BuildNodeIdForType(Type, QT.getTypePtr(), EmitRanges);
}

MaybeFew<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForType(const clang::QualType &QT) {
  assert(!QT.isNull());
  TypeSourceInfo *TSI = Context.getTrivialTypeSourceInfo(QT, SourceLocation());
  return BuildNodeIdForType(TSI->getTypeLoc(), EmitRanges::No);
}

MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::BuildNodeIdForType(
    const clang::TypeLoc &Type, const clang::Type *PT, EmitRanges EmitRanges) {
  MaybeFew<GraphObserver::NodeId> ID;
  const QualType QT = Type.getType();
  SourceRange SR = Type.getSourceRange();
  int64_t Key = ComputeKeyFromQualType(Context, QT, PT);
  const auto &Prev = TypeNodes.find(Key);
  bool TypeAlreadyBuilt = false;
  GraphObserver::Claimability Claimability =
      GraphObserver::Claimability::Claimable;
  if (Prev != TypeNodes.end()) {
    // If we're not trying to emit edges for constituent types, or if there's
    // no chance for us to do so because we lack source location information,
    // finish early.
    if (EmitRanges != EmitRanges::Yes ||
        !(SR.isValid() && SR.getBegin().isFileID())) {
      ID = Prev->second;
      return ID;
    }
    TypeAlreadyBuilt = true;
  }
  // We only care about leaves in the type hierarchy (eg, we shouldn't match
  // on Reference, but instead on LValueReference or RValueReference).
  switch (Type.getTypeLocClass()) {
  case TypeLoc::Qualified: {
    const auto &T = Type.castAs<QualifiedTypeLoc>();
    // TODO(zarko): ObjC tycons; embedded C tycons (address spaces).
    ID = BuildNodeIdForType(T.getUnqualifiedLoc(), PT, EmitRanges);
    if (TypeAlreadyBuilt) {
      break;
    }
    // Don't look down into type aliases. We'll have hit those during the
    // BuildNodeIdForType call above.
    // TODO(zarko): also add canonical edges (what do we call the edges?
    // 'expanded' seems reasonable).
    //   using ConstInt = const int;
    //   using CVInt1 = volatile ConstInt;
    if (T.getType().isLocalConstQualified()) {
      ID = ApplyBuiltinTypeConstructor("const", ID);
    }
    if (T.getType().isLocalRestrictQualified()) {
      ID = ApplyBuiltinTypeConstructor("restrict", ID);
    }
    if (T.getType().isLocalVolatileQualified()) {
      ID = ApplyBuiltinTypeConstructor("volatile", ID);
    }
  } break;
  case TypeLoc::Builtin: { // Leaf.
    const auto &T = Type.castAs<BuiltinTypeLoc>();
    if (TypeAlreadyBuilt) {
      break;
    }
    ID = Observer.getNodeIdForBuiltinType(T.getTypePtr()->getName(
        clang::PrintingPolicy(*Observer.getLangOptions())));
  } break;
  UNSUPPORTED_CLANG_TYPE(Complex);
  case TypeLoc::Pointer: {
    const auto &T = Type.castAs<PointerTypeLoc>();
    const auto *DT = dyn_cast<PointerType>(PT);
    auto PointeeID(BuildNodeIdForType(T.getPointeeLoc(),
                                      DT ? DT->getPointeeType().getTypePtr()
                                         : T.getPointeeLoc().getTypePtr(),
                                      EmitRanges));
    if (!PointeeID) {
      return PointeeID;
    }
    if (SR.isValid() && SR.getBegin().isFileID()) {
      SR.setEnd(clang::Lexer::getLocForEndOfToken(
          T.getStarLoc(), 0, /* offset from end of token */
          *Observer.getSourceManager(), *Observer.getLangOptions()));
    }
    if (TypeAlreadyBuilt) {
      break;
    }
    ID = ApplyBuiltinTypeConstructor("ptr", PointeeID);
  } break;
  UNSUPPORTED_CLANG_TYPE(BlockPointer);
  case TypeLoc::LValueReference: {
    const auto &T = Type.castAs<LValueReferenceTypeLoc>();
    const auto *DT = dyn_cast<LValueReferenceType>(PT);
    auto ReferentID(BuildNodeIdForType(T.getPointeeLoc(),
                                       DT ? DT->getPointeeType().getTypePtr()
                                          : T.getPointeeLoc().getTypePtr(),
                                       EmitRanges));
    if (!ReferentID) {
      return ReferentID;
    }
    if (SR.isValid() && SR.getBegin().isFileID()) {
      SR.setEnd(GetLocForEndOfToken(T.getAmpLoc()));
    }
    if (TypeAlreadyBuilt) {
      break;
    }
    ID = ApplyBuiltinTypeConstructor("lvr", ReferentID);
  } break;
  case TypeLoc::RValueReference: {
    const auto &T = Type.castAs<RValueReferenceTypeLoc>();
    const auto *DT = dyn_cast<RValueReferenceType>(PT);
    auto ReferentID(BuildNodeIdForType(T.getPointeeLoc(),
                                       DT ? DT->getPointeeType().getTypePtr()
                                          : T.getPointeeLoc().getTypePtr(),
                                       EmitRanges));
    if (!ReferentID) {
      return ReferentID;
    }
    if (SR.isValid() && SR.getBegin().isFileID()) {
      SR.setEnd(GetLocForEndOfToken(T.getAmpAmpLoc()));
    }
    if (TypeAlreadyBuilt) {
      break;
    }
    ID = ApplyBuiltinTypeConstructor("rvr", ReferentID);
  } break;
  UNSUPPORTED_CLANG_TYPE(MemberPointer);
  case TypeLoc::ConstantArray: {
    const auto &T = Type.castAs<ConstantArrayTypeLoc>();
    const auto *DT = dyn_cast<ConstantArrayType>(PT);
    auto ElementID(BuildNodeIdForType(T.getElementLoc(),
                                      DT ? DT->getElementType().getTypePtr()
                                         : T.getElementLoc().getTypePtr(),
                                      EmitRanges));
    if (!ElementID) {
      return ElementID;
    }
    if (TypeAlreadyBuilt) {
      break;
    }
    // TODO(zarko): Record size expression.
    ID = ApplyBuiltinTypeConstructor("carr", ElementID);
  } break;
  UNSUPPORTED_CLANG_TYPE(IncompleteArray);
  UNSUPPORTED_CLANG_TYPE(VariableArray);
  UNSUPPORTED_CLANG_TYPE(DependentSizedArray);
  UNSUPPORTED_CLANG_TYPE(DependentSizedExtVector);
  UNSUPPORTED_CLANG_TYPE(Vector);
  UNSUPPORTED_CLANG_TYPE(ExtVector);
  case TypeLoc::FunctionProto: {
    const auto &T = Type.castAs<FunctionProtoTypeLoc>();
    const auto *FT = cast<clang::FunctionProtoType>(Type.getType());
    const auto *DT = dyn_cast<FunctionProtoType>(PT);
    std::vector<GraphObserver::NodeId> NodeIds;
    std::vector<const GraphObserver::NodeId *> NodeIdPtrs;
    auto ReturnType(BuildNodeIdForType(T.getReturnLoc(),
                                       DT ? DT->getReturnType().getTypePtr()
                                          : T.getReturnLoc().getTypePtr(),
                                       EmitRanges));
    if (!ReturnType) {
      return ReturnType;
    }
    NodeIds.push_back(ReturnType.primary());
    unsigned Params = T.getNumParams();
    for (unsigned P = 0; P < Params; ++P) {
      MaybeFew<GraphObserver::NodeId> ParmType;
      if (const ParmVarDecl *PVD = T.getParam(P)) {
        const auto *DPT = DT && P < DT->getNumParams()
                              ? DT->getParamType(P).getTypePtr()
                              : nullptr;
        ParmType = BuildNodeIdForType(
            PVD->getTypeSourceInfo()->getTypeLoc(),
            DPT ? DPT : PVD->getTypeSourceInfo()->getTypeLoc().getTypePtr(),
            EmitRanges);
      } else {
        ParmType = BuildNodeIdForType(DT && P < DT->getNumParams()
                                          ? DT->getParamType(P)
                                          : FT->getParamType(P));
      }
      if (!ParmType) {
        return ParmType;
      }
      NodeIds.push_back(ParmType.primary());
    }
    for (size_t I = 0; I < NodeIds.size(); ++I) {
      NodeIdPtrs.push_back(&NodeIds[I]);
    }
    const char *Tycon = T.getTypePtr()->isVariadic() ? "fnvararg" : "fn";
    if (!TypeAlreadyBuilt) {
      ID = Observer.recordTappNode(Observer.getNodeIdForBuiltinType(Tycon),
                                   NodeIdPtrs);
    }
  } break;
  case TypeLoc::FunctionNoProto:
    if (!TypeAlreadyBuilt) {
      const auto &T = Type.castAs<FunctionNoProtoTypeLoc>();
      ID = Observer.getNodeIdForBuiltinType("knrfn");
    }
    break;
  UNSUPPORTED_CLANG_TYPE(UnresolvedUsing);
  case TypeLoc::Paren: {
    const auto &T = Type.castAs<ParenTypeLoc>();
    const auto *DT = dyn_cast<ParenType>(PT);
    ID =
        BuildNodeIdForType(T.getInnerLoc(), DT ? DT->getInnerType().getTypePtr()
                                               : T.getInnerLoc().getTypePtr(),
                           EmitRanges);
    EmitRanges = IndexerASTVisitor::EmitRanges::No;
  } break;
  case TypeLoc::Typedef: {
    // TODO(zarko): Return canonicalized versions as non-primary elements of
    // the MaybeFew.
    const auto &T = Type.castAs<TypedefTypeLoc>();
    GraphObserver::NameId AliasID(BuildNameIdForDecl(T.getTypedefNameDecl()));
    // We're retrieving the type of an alias here, so we shouldn't thread
    // through the deduced type.
    auto AliasedTypeID(BuildNodeIdForType(
        T.getTypedefNameDecl()->getTypeSourceInfo()->getTypeLoc(),
        IndexerASTVisitor::EmitRanges::No));
    if (!AliasedTypeID) {
      return AliasedTypeID;
    }
    ID = TypeAlreadyBuilt
             ? Observer.nodeIdForTypeAliasNode(AliasID, AliasedTypeID.primary())
             : Observer.recordTypeAliasNode(AliasID, AliasedTypeID.primary());
  } break;
  UNSUPPORTED_CLANG_TYPE(Adjusted);
  UNSUPPORTED_CLANG_TYPE(Decayed);
  UNSUPPORTED_CLANG_TYPE(TypeOfExpr);
  UNSUPPORTED_CLANG_TYPE(TypeOf);
  case TypeLoc::Decltype:
    if (!TypeAlreadyBuilt) {
      const auto &T = Type.castAs<DecltypeTypeLoc>();
      const auto *DT = dyn_cast<DecltypeType>(PT);
      ID = BuildNodeIdForType(DT ? DT->getUnderlyingType()
                                 : T.getTypePtr()->getUnderlyingType());
    }
    break;
  UNSUPPORTED_CLANG_TYPE(UnaryTransform);
  case TypeLoc::Record: { // Leaf.
    const auto &T = Type.castAs<RecordTypeLoc>();
    RecordDecl *Decl = T.getDecl();
    if (const auto *Spec = dyn_cast<ClassTemplateSpecializationDecl>(Decl)) {
      // Clang doesn't appear to construct TemplateSpecialization
      // types for non-dependent specializations, instead representing
      // them as ClassTemplateSpecializationDecls directly.
      clang::ClassTemplateDecl *SpecializedTemplateDecl =
          Spec->getSpecializedTemplate();
      // Link directly to the defn if we have it; otherwise use tnominal.
      if (SpecializedTemplateDecl->getTemplatedDecl()->getDefinition()) {
        Claimability = GraphObserver::Claimability::Unclaimable;
      }
      GraphObserver::NodeId TemplateName =
          Claimability == GraphObserver::Claimability::Unclaimable
              ? BuildNodeIdForDecl(SpecializedTemplateDecl)
              : Observer.recordNominalTypeNode(
                    BuildNameIdForDecl(SpecializedTemplateDecl));
      const auto &TAL = Spec->getTemplateArgs();
      std::vector<GraphObserver::NodeId> TemplateArgs;
      TemplateArgs.reserve(TAL.size());
      std::vector<const GraphObserver::NodeId *> TemplateArgsPtrs;
      TemplateArgsPtrs.resize(TAL.size(), nullptr);
      for (unsigned A = 0, AE = TAL.size(); A != AE; ++A) {
        if (auto ArgA =
                BuildNodeIdForTemplateArgument(TAL[A], Spec->getLocation())) {
          TemplateArgs.push_back(ArgA.primary());
          TemplateArgsPtrs[A] = &TemplateArgs[A];
        } else {
          return ArgA;
        }
      }
      if (!TypeAlreadyBuilt) {
        ID = Observer.recordTappNode(TemplateName, TemplateArgsPtrs);
      }
    } else {
      if (RecordDecl *Defn = Decl->getDefinition()) {
        Claimability = GraphObserver::Claimability::Unclaimable;
        if (!TypeAlreadyBuilt) {
          // Special-case linking to a defn instead of using a tnominal.
          if (const auto *RD = dyn_cast<CXXRecordDecl>(Defn)) {
            if (const auto *CTD = RD->getDescribedClassTemplate()) {
              // Link to the template binder, not the internal class.
              ID = BuildNodeIdForDecl(CTD);
            } else {
              // This is an ordinary CXXRecordDecl.
              ID = BuildNodeIdForDecl(Defn);
            }
          } else {
            // This is a non-CXXRecordDecl, so it can't be templated.
            ID = BuildNodeIdForDecl(Defn);
          }
        }
      } else {
        // Thanks to the ODR, we shouldn't record multiple nominal type nodes
        // for the same TU: given distinct names, NameIds will be distinct,
        // there may be only one definition bound to each name, and we
        // memoize the NodeIds we give to types.
        if (!TypeAlreadyBuilt) {
          ID = Observer.recordNominalTypeNode(BuildNameIdForDecl(Decl));
        }
      }
    }
  } break;
  case TypeLoc::Enum: { // Leaf.
    const auto &T = Type.castAs<EnumTypeLoc>();
    EnumDecl *Decl = T.getDecl();
    if (!TypeAlreadyBuilt) {
      if (EnumDecl *Defn = Decl->getDefinition()) {
        Claimability = GraphObserver::Claimability::Unclaimable;
        ID = BuildNodeIdForDecl(Defn);
      } else {
        ID = Observer.recordNominalTypeNode(BuildNameIdForDecl(Decl));
      }
    } else {
      if (Decl->getDefinition()) {
        Claimability = GraphObserver::Claimability::Unclaimable;
      }
    }
  } break;
  case TypeLoc::Elaborated: {
    // This one wraps a qualified (via 'struct S' | 'N::M::type') type
    // reference.
    const auto &T = Type.castAs<ElaboratedTypeLoc>();
    const auto *DT = dyn_cast<ElaboratedType>(PT);
    ID = BuildNodeIdForType(T.getNamedTypeLoc(),
                            DT ? DT->getNamedType().getTypePtr()
                               : T.getNamedTypeLoc().getTypePtr(),
                            EmitRanges);
    // Don't link 'struct'.
    EmitRanges = EmitRanges::No;
    // TODO(zarko): Add anchors for parts of the NestedNameSpecifier.
    // We'll add an anchor for all the Elaborated type, though; otherwise decls
    // like `typedef B::C tdef;` will only anchor `C` instead of `B::C`.
  } break;
  UNSUPPORTED_CLANG_TYPE(Attributed);
  // Either the `TemplateTypeParm` will link directly to a relevant
  // `TemplateTypeParmDecl` or (particularly in the case of canonicalized types)
  // we will find the Decl in the `TypeContext` according to the parameter's
  // depth and index.
  case TypeLoc::TemplateTypeParm: { // Leaf.
    // Depths count from the outside-in; each Template*ParmDecl has only
    // one possible (depth, index).
    const auto *TypeParm = cast<TemplateTypeParmType>(Type.getTypePtr());
    const auto *TD = TypeParm->getDecl();
    if (!IgnoreUnimplemented) {
      // TODO(zarko): Remove sanity checks. If things go poorly here,
      // dump with DumpTypeContext(T->getDepth(), T->getIndex());
      assert(TypeParm->getDepth() < TypeContext.size() &&
             "Decl for type parameter missing from context.");
      assert(TypeParm->getIndex() < TypeContext[TypeParm->getDepth()]->size() &&
             "Decl for type parameter missing at specified depth.");
      const auto *ND =
          TypeContext[TypeParm->getDepth()]->getParam(TypeParm->getIndex());
      TD = cast<TemplateTypeParmDecl>(ND);
      assert(TypeParm->getDecl() == nullptr || TypeParm->getDecl() == TD);
    } else if (!TD) {
      return None();
    }
    ID = BuildNodeIdForDecl(TD);
  } break;
  // "Within an instantiated template, all template type parameters have been
  // replaced with these. They are used solely to record that a type was
  // originally written as a template type parameter; therefore they are
  // never canonical."
  case TypeLoc::SubstTemplateTypeParm: {
    const auto &T = Type.castAs<SubstTemplateTypeParmTypeLoc>();
    const SubstTemplateTypeParmType *STTPT = T.getTypePtr();
    // TODO(zarko): Record both the replaced parameter and the replacement type.
    assert(!STTPT->getReplacementType().isNull());
    ID = BuildNodeIdForType(STTPT->getReplacementType());
  } break;
  // "When a pack expansion in the source code contains multiple parameter packs
  // and those parameter packs correspond to different levels of template
  // parameter lists, this type node is used to represent a template type
  // parameter pack from an outer level, which has already had its argument pack
  // substituted but that still lives within a pack expansion that itself
  // could not be instantiated. When actually performing a substitution into
  // that pack expansion (e.g., when all template parameters have corresponding
  // arguments), this type will be replaced with the SubstTemplateTypeParmType
  // at the current pack substitution index."
  UNSUPPORTED_CLANG_TYPE(SubstTemplateTypeParmPack);
  case TypeLoc::TemplateSpecialization: {
    // This refers to a particular class template, type alias template,
    // or template template parameter. Non-dependent template specializations
    // appear as different types.
    const auto &T = Type.castAs<TemplateSpecializationTypeLoc>();
    // TODO(zarko): Consider linking directly to the instantiation or partial
    // specialization here if one has been created. These can be found with
    // ClassTemplateDecl::{findSpecialization, findPartialSpecialization}.
    auto TemplateName = BuildNodeIdForTemplateName(
        T.getTypePtr()->getTemplateName(), T.getTemplateNameLoc());
    if (!TemplateName) {
      return TemplateName;
    }
    std::vector<GraphObserver::NodeId> TemplateArgs;
    TemplateArgs.reserve(T.getNumArgs());
    std::vector<const GraphObserver::NodeId *> TemplateArgsPtrs;
    TemplateArgsPtrs.resize(T.getNumArgs(), nullptr);
    for (unsigned A = 0, AE = T.getNumArgs(); A != AE; ++A) {
      if (auto ArgA =
              BuildNodeIdForTemplateArgument(T.getArgLoc(A), EmitRanges)) {
        TemplateArgs.push_back(ArgA.primary());
        TemplateArgsPtrs[A] = &TemplateArgs[A];
      } else {
        return ArgA;
      }
    }
    if (!TypeAlreadyBuilt) {
      ID = Observer.recordTappNode(TemplateName.primary(), TemplateArgsPtrs);
    }
  } break;
  case TypeLoc::Auto: {
    const auto *DT = dyn_cast<AutoType>(PT);
    QualType DeducedQT = DT->getDeducedType();
    if (DeducedQT.isNull()) {
      const auto &AutoT = Type.castAs<AutoTypeLoc>();
      const auto *T = AutoT.getTypePtr();
      DeducedQT = T->getDeducedType();
      if (DeducedQT.isNull()) {
        // We still need to come up with a name here--it's more useful than
        // returning None, since we might be down a branch of some structural
        // type. We might also have an unconstrained type variable,
        // as with `auto foo();` with no definition.
        ID = Observer.getNodeIdForBuiltinType("auto");
        break;
      }
    }
    ID = BuildNodeIdForType(DeducedQT);
  } break;
  case TypeLoc::InjectedClassName: {
    const auto &T = Type.castAs<InjectedClassNameTypeLoc>();
    CXXRecordDecl *Decl = T.getDecl();
    if (!TypeAlreadyBuilt) { // Leaf.
      // TODO(zarko): Replace with logic that uses InjectedType.
      if (RecordDecl *Defn = Decl->getDefinition()) {
        Claimability = GraphObserver::Claimability::Unclaimable;
        if (const auto *RD = dyn_cast<CXXRecordDecl>(Defn)) {
          if (const auto *CTD = RD->getDescribedClassTemplate()) {
            // Link to the template binder, not the internal class.
            ID = BuildNodeIdForDecl(CTD);
          } else {
            // This is an ordinary CXXRecordDecl.
            ID = BuildNodeIdForDecl(Defn);
          }
        } else {
          // This is a non-CXXRecordDecl, so it can't be templated.
          ID = BuildNodeIdForDecl(Defn);
        }
      } else {
        ID = Observer.recordNominalTypeNode(BuildNameIdForDecl(Decl));
      }
    } else {
      if (Decl->getDefinition() != nullptr) {
        Claimability = GraphObserver::Claimability::Unclaimable;
      }
    }
  } break;
  case TypeLoc::DependentName:
    if (!TypeAlreadyBuilt) {
      const auto &T = Type.castAs<DependentNameTypeLoc>();
      const auto &NNS = T.getQualifierLoc();
      const auto &NameLoc = T.getNameLoc();
      ID = BuildNodeIdForDependentName(NNS, T.getTypePtr()->getIdentifier(),
                                       NameLoc, EmitRanges);
    }
    break;
  UNSUPPORTED_CLANG_TYPE(DependentTemplateSpecialization);
  UNSUPPORTED_CLANG_TYPE(PackExpansion);
  UNSUPPORTED_CLANG_TYPE(ObjCObject);
  UNSUPPORTED_CLANG_TYPE(ObjCInterface); // Leaf.
  UNSUPPORTED_CLANG_TYPE(ObjCObjectPointer);
  UNSUPPORTED_CLANG_TYPE(Atomic);
  default:
    // Reference, Array, Function
    assert(0 && "Incomplete pattern match on type or abstract class (?)");
  }
  if (TypeAlreadyBuilt) {
    ID = Prev->second;
  } else {
    TypeNodes[Key] = ID;
  }
  if (SR.isValid() && SR.getBegin().isFileID()) {
    // If this is an empty SourceRange, try to expand it.
    if (SR.getBegin() == SR.getEnd()) {
      SR = RangeForASTEntityFromSourceLocation(SR.getBegin());
    }
    if (EmitRanges == IndexerASTVisitor::EmitRanges::Yes) {
      auto RCC = RangeInCurrentContext(SR);
      ID.Iter([&](const GraphObserver::NodeId &I) {
        Observer.recordTypeSpellingLocation(RCC, I, Claimability);
      });
    }
  }
  return ID;
}

} // namespace kythe
