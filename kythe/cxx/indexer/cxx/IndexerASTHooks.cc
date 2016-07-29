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
#include "clang/AST/CommentLexer.h"
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
#include "clang/Sema/Lookup.h"

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
      CHECK_LE(Size, Target->size());
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
  CHECK(Node.getMemoizationData() != nullptr)
      << "Invariant broken: only nodes that support memoization may be "
         "used in the parent map.";
  if (!AllParents) {
    // We always need to run over the whole translation unit, as
    // hasAncestor can escape any subtree.
    // TODO(zarko): Is this relavant for naming?
    ProfileBlock block(Observer.getProfilingCallback(), "build_parent_map");
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
  CHECK(Loc != nullptr);

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
  CHECK(Start.isFileID());
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
    const MaybeFew<GraphObserver::Range> &R, const GraphObserver::NodeId &Id) {
  if (R) {
    Observer.recordDefinitionBindingRange(R.primary(), Id);
  }
}

void IndexerASTVisitor::RecordCallEdges(const GraphObserver::Range &Range,
                                        const GraphObserver::NodeId &Callee) {
  if (!BlameStack.empty()) {
    for (const auto &Caller : BlameStack.back()) {
      Observer.recordCallEdge(Range, Caller, Callee);
    }
  }
}

/// Decides whether `Tok` can be used to quote an identifier.
static bool TokenQuotesIdentifier(const clang::SourceManager &SM,
                                  const clang::Token &Tok) {
  switch (Tok.getKind()) {
  case tok::TokenKind::pipe:
    return true;
  case tok::TokenKind::unknown: {
    bool Invalid = false;
    if (const char *TokChar =
            SM.getCharacterData(Tok.getLocation(), &Invalid)) {
      if (!Invalid && Tok.getLength() > 0) {
        return *TokChar == '`';
      }
    }
    return false;
  }
  default:
    return false;
  }
}

/// \brief Attempt to find the template parameters bound immediately by `DC`.
/// \return null if no parameters could be found.
static clang::TemplateParameterList *
GetTypeParameters(const clang::DeclContext *DC) {
  if (!DC) {
    return nullptr;
  }
  if (const auto *TemplateContext = dyn_cast<clang::TemplateDecl>(DC)) {
    return TemplateContext->getTemplateParameters();
  } else if (const auto *CTPSD =
                 dyn_cast<ClassTemplatePartialSpecializationDecl>(DC)) {
    return CTPSD->getTemplateParameters();
  } else if (const auto *RD = dyn_cast<CXXRecordDecl>(DC)) {
    if (const auto *TD = RD->getDescribedClassTemplate()) {
      return TD->getTemplateParameters();
    }
  } else if (const auto *VTPSD =
                 dyn_cast<VarTemplatePartialSpecializationDecl>(DC)) {
    return VTPSD->getTemplateParameters();
  } else if (const auto *VD = dyn_cast<VarDecl>(DC)) {
    if (const auto *TD = VD->getDescribedVarTemplate()) {
      return TD->getTemplateParameters();
    }
  } else if (const auto *AD = dyn_cast<TypeAliasDecl>(DC)) {
    if (const auto *TD = AD->getDescribedAliasTemplate()) {
      return TD->getTemplateParameters();
    }
  }
  return nullptr;
}

/// Make sure `DC` won't cause Sema::LookupQualifiedName to fail an assertion.
static bool IsContextSafeForLookup(const clang::DeclContext *DC) {
  if (const auto *TD = dyn_cast<clang::TagDecl>(DC)) {
    return TD->isCompleteDefinition() || TD->isBeingDefined();
  }
  return DC->isDependentContext() || !isa<clang::LinkageSpecDecl>(DC);
}

/// An in-flight possible lookup result used to approximate qualified lookup.
struct PossibleLookup {
  clang::LookupResult Result;
  const clang::DeclContext *Context;
};

/// Greedily consumes tokens to try and find the longest qualified name that
/// results in a nonempty lookup result.
class PossibleLookups {
public:
  /// \param RC The most specific context to start searching in.
  /// \param TC The translation unit context.
  PossibleLookups(clang::ASTContext &C, clang::Sema &S,
                  const clang::DeclContext *RC, const clang::DeclContext *TC)
      : Context(C), Sema(S), RootContext(RC), TUContext(TC) {
    StartIdentifier();
  }

  /// Describes what happened after we added the last token to the lookup state.
  enum class LookupResult {
    Done,    ///< Lookup is terminated; don't add any more tokens.
    Updated, ///< The last token narrowed down the lookup.
    Progress ///< The last token didn't narrow anything down.
  };

  /// Try to use `Tok` to narrow down the lookup.
  LookupResult AdvanceLookup(const clang::Token &Tok) {
    if (Tok.is(clang::tok::coloncolon)) {
      // This flag only matters if the possible lookup list is empty.
      // Just drop ::s on the floor otherwise.
      ForceTUContext = true;
      return LookupResult::Progress;
    } else if (Tok.is(clang::tok::raw_identifier)) {
      bool Invalid = false;
      const char *TokChar = Context.getSourceManager().getCharacterData(
          Tok.getLocation(), &Invalid);
      if (Invalid || !TokChar) {
        return LookupResult::Done;
      }
      llvm::StringRef TokText(TokChar, Tok.getLength());
      const auto &Id = Context.Idents.get(TokText);
      clang::DeclarationNameInfo DeclName(
          Context.DeclarationNames.getIdentifier(&Id), Tok.getLocation());
      if (Lookups.empty()) {
        return StartLookups(DeclName);
      } else {
        return AdvanceLookups(DeclName);
      }
    }
    return LookupResult::Done;
  }

  /// Start a new identifier.
  void StartIdentifier() {
    Lookups.clear();
    ForceTUContext = false;
  }

  /// Retrieve a vector of plausible lookup results with the most specific
  /// ones first.
  const std::vector<PossibleLookup> &LookupState() { return Lookups; }

private:
  /// Start a new lookup from `DeclName`.
  LookupResult StartLookups(const clang::DeclarationNameInfo &DeclName) {
    CHECK(Lookups.empty());
    const clang::DeclContext *Context =
        ForceTUContext ? TUContext : RootContext;
    do {
      clang::LookupResult FirstLookup(
          Sema, DeclName, clang::Sema::LookupNameKind::LookupAnyName);
      FirstLookup.suppressDiagnostics();
      if (IsContextSafeForLookup(Context) &&
          Sema.LookupQualifiedName(
              FirstLookup, const_cast<clang::DeclContext *>(Context), false)) {
        Lookups.push_back({std::move(FirstLookup), Context});
      } else {
        CHECK(FirstLookup.empty());
        // We could be looking at a (type) parameter here. Note that this
        // may still be part of a qualified (dependent) name.
        if (const auto *FunctionContext =
                dyn_cast_or_null<clang::FunctionDecl>(Context)) {
          for (const auto &Param : FunctionContext->params()) {
            if (Param->getDeclName() == DeclName.getName()) {
              clang::LookupResult DerivedResult(clang::LookupResult::Temporary,
                                                FirstLookup);
              DerivedResult.suppressDiagnostics();
              DerivedResult.addDecl(Param);
              Lookups.push_back({std::move(DerivedResult), Context});
            }
          }
        } else if (const auto *TemplateParams = GetTypeParameters(Context)) {
          for (const auto &TParam : *TemplateParams) {
            if (TParam->getDeclName() == DeclName.getName()) {
              clang::LookupResult DerivedResult(clang::LookupResult::Temporary,
                                                FirstLookup);
              DerivedResult.suppressDiagnostics();
              DerivedResult.addDecl(TParam);
              Lookups.push_back({std::move(DerivedResult), Context});
            }
          }
        }
      }
      Context = Context->getParent();
    } while (Context != nullptr);
    return Lookups.empty() ? LookupResult::Done : LookupResult::Updated;
  }
  /// Continue each in-flight lookup using `DeclName`.
  LookupResult AdvanceLookups(const clang::DeclarationNameInfo &DeclName) {
    // Try to advance each lookup we have stored.
    std::vector<PossibleLookup> ResultLookups;
    for (auto &Lookup : Lookups) {
      for (const clang::NamedDecl *Result : Lookup.Result) {
        const auto *Context = dyn_cast<clang::DeclContext>(Result);
        if (!Context) {
          Context = Lookup.Context;
        }
        clang::LookupResult NextResult(
            Sema, DeclName, clang::Sema::LookupNameKind::LookupAnyName);
        NextResult.suppressDiagnostics();
        if (IsContextSafeForLookup(Context) &&
            Sema.LookupQualifiedName(
                NextResult, const_cast<clang::DeclContext *>(Context), false)) {
          ResultLookups.push_back({std::move(NextResult), Context});
        }
      }
    }
    if (!ResultLookups.empty()) {
      Lookups = std::move(ResultLookups);
      return LookupResult::Updated;
    } else {
      return LookupResult::Done;
    }
  }
  /// All the plausible lookup results so far. This is sorted with the most
  /// specific results first (i.e., the ones that came from the closest
  /// DeclContext).
  std::vector<PossibleLookup> Lookups;
  /// The ASTContext we're under.
  clang::ASTContext &Context;
  /// The Sema instance we're using.
  clang::Sema &Sema;
  /// The nearest declaration context.
  const clang::DeclContext *RootContext;
  /// The translation unit's declaration context.
  const clang::DeclContext *TUContext;
  /// Set if the current lookup is unambiguously in TUContext.
  bool ForceTUContext = false;
};

/// The state for heuristic parsing of identifiers in comment bodies.
enum class RefParserState {
  WaitingForMark, ///< We're waiting for something that might denote an
                  ///< embedded identifier.
  SawCommand      ///< We saw something Clang identifies as a comment command.
};

/// Adds markup to Text to define anchor locations and sorts Anchors
/// accordingly.
void InsertAnchorMarks(std::string &Text, std::vector<MiniAnchor> &Anchors) {
  // Drop empty or negative-length anchors.
  Anchors.erase(
      std::remove_if(Anchors.begin(), Anchors.end(),
                     [](const MiniAnchor &A) { return A.Begin >= A.End; }),
      Anchors.end());
  std::sort(Anchors.begin(), Anchors.end(),
            [](const MiniAnchor &A, const MiniAnchor &B) {
              return std::tie(A.Begin, B.End) < std::tie(B.Begin, A.End);
            });
  std::string NewText;
  NewText.reserve(Text.size() + Anchors.size() * 2);
  auto NextAnchor = Anchors.begin();
  std::multiset<size_t> EndLocations;
  for (size_t C = 0; C < Text.size(); ++C) {
    while (!EndLocations.empty() && *(EndLocations.begin()) == C) {
      NewText.push_back(']');
      EndLocations.erase(EndLocations.begin());
    }
    for (; NextAnchor != Anchors.end() && C == NextAnchor->Begin;
         ++NextAnchor) {
      NewText.push_back('[');
      EndLocations.insert(NextAnchor->End);
    }
    if (Text[C] == '[' || Text[C] == ']' || Text[C] == '\\') {
      // Escape [, ], and \.
      NewText.push_back('\\');
    }
    NewText.push_back(Text[C]);
  }
  NewText.resize(NewText.size() + EndLocations.size(), ']');
  Text.swap(NewText);
}

bool IndexerASTVisitor::VisitDecl(const clang::Decl *Decl) {
  const auto &SM = *Observer.getSourceManager();
  const auto *Comment = Context.getRawCommentForDeclNoCache(Decl);
  if (!Comment) {
    // Fast path: if there are no attached documentation comments, bail.
    return true;
  }
  const auto *DC = dyn_cast<DeclContext>(Decl);
  if (!DC) {
    DC = Decl->getDeclContext();
    if (!DC) {
      DC = Context.getTranslationUnitDecl();
    }
  }
  std::string Text = Comment->getRawText(SM).str();
  std::string StrippedRawText;
  std::map<clang::SourceLocation, size_t> OffsetsInStrippedRawText;

  std::vector<MiniAnchor> StrippedRawTextAnchors;
  clang::comments::Lexer Lexer(Context.getAllocator(), Context.getDiagnostics(),
                               Context.getCommentCommandTraits(),
                               Comment->getSourceRange().getBegin(),
                               Text.data(), Text.data() + Text.size());
  clang::comments::Token Tok;
  std::vector<clang::Token> IdentifierTokens;
  PossibleLookups Lookups(Context, Sema, DC, Context.getTranslationUnitDecl());
  auto RecordDocDeclUse = [&](const clang::SourceRange &Range,
                              const GraphObserver::NodeId &ResultId) {
    if (auto RCC = ExplicitRangeInCurrentContext(Range)) {
      Observer.recordDeclUseLocationInDocumentation(RCC.primary(), ResultId);
    }
    auto RawLoc = OffsetsInStrippedRawText.lower_bound(Range.getBegin());
    if (RawLoc != OffsetsInStrippedRawText.end()) {
      // This is a single token (so we won't span multiple stripped ranges).
      size_t StrippedBegin = RawLoc->second +
                             Range.getBegin().getRawEncoding() -
                             RawLoc->first.getRawEncoding();
      size_t StrippedLength =
          Range.getEnd().getRawEncoding() - Range.getBegin().getRawEncoding();
      if (StrippedLength != 0) {
        StrippedRawTextAnchors.emplace_back(MiniAnchor{
            StrippedBegin, StrippedBegin + StrippedLength, ResultId});
      }
    }
  };
  // Attribute all tokens on [FirstToken,LastToken] to every possible lookup
  // result inside PossibleLookups.
  auto HandleLookupResult = [&](int FirstToken, int LastToken) {
    for (const auto &Results : Lookups.LookupState()) {
      for (auto Result : Results.Result) {
        auto ResultId = BuildNodeIdForDecl(Result);
        for (int Token = FirstToken; Token <= LastToken; ++Token) {
          RecordDocDeclUse(
              clang::SourceRange(IdentifierTokens[Token].getLocation(),
                                 IdentifierTokens[Token].getEndLoc()),
              ResultId);
        }
      }
    }
  };
  auto State = RefParserState::WaitingForMark;
  // FoundEndingDelimiter will be called whenever we're sure that no more
  // tokens can be added to IdentifierTokens that could be related to the same
  // identifier. (IdentifierTokens might still contain multiple identifiers,
  // as in the doc text "`foo`, `bar`, or `baz`".)
  // TODO(zarko): Deal with template arguments (`foo<int>::bar`), balanced
  // delimiters (`foo::bar(int, c<int>)`), and dtors (which are troubling
  // because they have ~s out in front). The goal here is not to completely
  // reimplement a C/C++ parser, but instead to build just enough to make good
  // guesses.
  auto FoundEndingDelimiter = [&]() {
    int IdentifierStart = -1;
    int CurrentIndex = 0;
    if (State == RefParserState::SawCommand) {
      // Start as though we've seen a quote and consume tokens until we're not
      // making progress.
      IdentifierStart = 0;
      Lookups.StartIdentifier();
    }
    for (const auto &Tok : IdentifierTokens) {
      if (IdentifierStart >= 0) {
        if (Lookups.AdvanceLookup(Tok) == PossibleLookups::LookupResult::Done) {
          // We can't match an identifier using this token. Either it's the
          // matching close-quote or it's some other weirdness.
          if (IdentifierStart != CurrentIndex) {
            HandleLookupResult(IdentifierStart, CurrentIndex - 1);
          }
          IdentifierStart = -1;
        }
      } else if (TokenQuotesIdentifier(SM, Tok)) {
        // We found a signal to start an identifier.
        IdentifierStart = CurrentIndex + 1;
        Lookups.StartIdentifier();
      }
      ++CurrentIndex;
    }
    State = RefParserState::WaitingForMark;
    IdentifierTokens.clear();
  };
  do {
    Lexer.lex(Tok);
    unsigned offset = Tok.getLocation().getRawEncoding() -
                      Comment->getSourceRange().getBegin().getRawEncoding();
    OffsetsInStrippedRawText.insert(
        std::make_pair(Tok.getLocation(), StrippedRawText.size()));
    StrippedRawText.append(Text.substr(offset, Tok.getLength()));
    switch (Tok.getKind()) {
    case clang::comments::tok::text: {
      auto TokText = Tok.getText().str();
      clang::Lexer LookupLexer(Tok.getLocation(), *Observer.getLangOptions(),
                               TokText.data(), TokText.data(),
                               TokText.data() + TokText.size());
      do {
        IdentifierTokens.emplace_back();
        LookupLexer.LexFromRawLexer(IdentifierTokens.back());
      } while (!IdentifierTokens.back().is(clang::tok::eof));
      IdentifierTokens.pop_back();
    } break;
    case clang::comments::tok::newline:
      break;
    case clang::comments::tok::unknown_command:
    case clang::comments::tok::backslash_command:
    case clang::comments::tok::at_command:
      FoundEndingDelimiter();
      State = RefParserState::SawCommand;
      break;
    case clang::comments::tok::verbatim_block_begin:
    case clang::comments::tok::verbatim_block_line:
    case clang::comments::tok::verbatim_block_end:
    case clang::comments::tok::verbatim_line_name:
    case clang::comments::tok::verbatim_line_text:
    case clang::comments::tok::html_start_tag:
    case clang::comments::tok::html_ident:
    case clang::comments::tok::html_equals:
    case clang::comments::tok::html_quoted_string:
    case clang::comments::tok::html_greater:
    case clang::comments::tok::html_slash_greater:
    case clang::comments::tok::html_end_tag:
    case clang::comments::tok::eof:
      FoundEndingDelimiter();
      break;
    }
  } while (Tok.getKind() != clang::comments::tok::eof);
  InsertAnchorMarks(StrippedRawText, StrippedRawTextAnchors);
  std::vector<GraphObserver::NodeId> LinkNodes;
  LinkNodes.reserve(StrippedRawTextAnchors.size());
  for (const auto &Anchor : StrippedRawTextAnchors) {
    LinkNodes.push_back(Anchor.AnchoredTo);
  }
  // Attribute the raw comment text to its associated decl.
  if (auto RCC = ExplicitRangeInCurrentContext(Comment->getSourceRange())) {
    if (const auto *DC = dyn_cast_or_null<DeclContext>(Decl)) {
      if (auto DCID = BuildNodeIdForDeclContext(DC)) {
        Observer.recordDocumentationRange(RCC.primary(), DCID.primary());
        Observer.recordDocumentationText(DCID.primary(), StrippedRawText,
                                         LinkNodes);
      }
      if (const auto *CTPSD =
              dyn_cast_or_null<ClassTemplatePartialSpecializationDecl>(Decl)) {
        auto NodeId = BuildNodeIdForDecl(CTPSD);
        Observer.recordDocumentationRange(RCC.primary(), NodeId);
        Observer.recordDocumentationText(NodeId, StrippedRawText, LinkNodes);
      }
      if (const auto *FD = dyn_cast_or_null<FunctionDecl>(Decl)) {
        if (const auto *FTD = FD->getDescribedFunctionTemplate()) {
          auto NodeId = BuildNodeIdForDecl(FTD);
          Observer.recordDocumentationRange(RCC.primary(), NodeId);
          Observer.recordDocumentationText(NodeId, StrippedRawText, LinkNodes);
        }
      }
    } else {
      if (const auto *VD = dyn_cast_or_null<VarDecl>(Decl)) {
        if (const auto *VTD = VD->getDescribedVarTemplate()) {
          auto NodeId = BuildNodeIdForDecl(VTD);
          Observer.recordDocumentationRange(RCC.primary(), NodeId);
          Observer.recordDocumentationText(NodeId, StrippedRawText, LinkNodes);
        }
      } else if (const auto *AD = dyn_cast_or_null<TypeAliasDecl>(Decl)) {
        if (const auto *TATD = AD->getDescribedAliasTemplate()) {
          auto NodeId = BuildNodeIdForDecl(TATD);
          Observer.recordDocumentationRange(RCC.primary(), NodeId);
          Observer.recordDocumentationText(NodeId, StrippedRawText, LinkNodes);
        }
      }
      auto NodeId = BuildNodeIdForDecl(Decl);
      Observer.recordDocumentationRange(RCC.primary(), NodeId);
      Observer.recordDocumentationText(NodeId, StrippedRawText, LinkNodes);
    }
  }
  return true;
}

/// \brief Returns whether `Ctor` would override the in-class initializer for
/// `Field`.
static bool
ConstructorOverridesInitializer(const clang::CXXConstructorDecl *Ctor,
                                const clang::FieldDecl *Field) {
  const clang::FunctionDecl *CtorDefn = nullptr;
  if (Ctor->isDefined(CtorDefn) &&
      (Ctor = dyn_cast_or_null<CXXConstructorDecl>(CtorDefn))) {
    for (const auto *Init : Ctor->inits()) {
      if (Init->getMember() == Field && !Init->isInClassMemberInitializer()) {
        return true;
      }
    }
  }
  return false;
}

bool IndexerASTVisitor::TraverseDecl(clang::Decl *Decl) {
  GraphObserver::Delimiter Del(Observer);
  // For clang::FunctionDecl and all subclasses thereof push blame data.
  if (auto *FD = dyn_cast_or_null<clang::FunctionDecl>(Decl)) {
    if (unsigned BuiltinID = FD->getBuiltinID()) {
      if (!Observer.getPreprocessor()->getBuiltinInfo().isPredefinedLibFunction(
              BuiltinID)) {
        // These builtins act weirdly (by, e.g., defining themselves inside the
        // macro context where they first appeared, but then also in the global
        // namespace). Don't traverse them.
        return true;
      }
    }
    auto R = RestoreStack(BlameStack);
    auto S = RestoreStack(RangeContext);

    if (FD->isTemplateInstantiation() &&
        FD->getTemplateSpecializationKind() !=
            clang::TSK_ExplicitSpecialization) {
      // Explicit specializations have ranges.
      if (const auto RangeId = BuildNodeIdForDeclContext(FD)) {
        RangeContext.push_back(RangeId.primary());
      } else {
        RangeContext.push_back(BuildNodeIdForDecl(FD));
      }
    }
    if (const auto BlameId = BuildNodeIdForDeclContext(FD)) {
      BlameStack.push_back(SomeNodes(1, BlameId.primary()));
    } else {
      BlameStack.push_back(SomeNodes(1, BuildNodeIdForDecl(FD)));
    }

    // Dispatch the remaining logic to the base class TraverseDecl() which will
    // call TraverseX(X*) for the most-derived X.
    if (!Observer.lossy_claiming() ||
        Observer.claimNode(BuildNodeIdForDecl(FD))) {
      return RecursiveASTVisitor::TraverseDecl(FD);
    } else {
      return true;
    }
  } else if (auto *ID = dyn_cast_or_null<clang::FieldDecl>(Decl)) {
    if (ID->hasInClassInitializer()) {
      // Blame calls from in-class initializers I on all ctors C of the
      // containing class so long as C does not have its own initializer for
      // I's field.
      auto R = RestoreStack(BlameStack);
      if (auto *CR = dyn_cast_or_null<clang::CXXRecordDecl>(ID->getParent())) {
        if ((CR = CR->getDefinition())) {
          SomeNodes Ctors;
          auto TryCtor = [&](const clang::CXXConstructorDecl *Ctor) {
            if (!ConstructorOverridesInitializer(Ctor, ID)) {
              if (const auto BlameId = BuildNodeIdForDeclContext(Ctor)) {
                Ctors.push_back(BlameId.primary());
              }
            }
          };
          for (const auto *SubDecl : CR->decls()) {
            if (const auto *Ctor =
                    dyn_cast_or_null<CXXConstructorDecl>(SubDecl)) {
              TryCtor(Ctor);
            } else if (const auto *Templ =
                           dyn_cast_or_null<FunctionTemplateDecl>(SubDecl)) {
              if (const auto *Ctor = dyn_cast_or_null<CXXConstructorDecl>(
                      Templ->getTemplatedDecl())) {
                TryCtor(Ctor);
              }
              for (const auto *Spec : Templ->specializations()) {
                if (const auto *Ctor =
                        dyn_cast_or_null<CXXConstructorDecl>(Spec)) {
                  TryCtor(Ctor);
                }
              }
            }
          }
          if (!Ctors.empty()) {
            BlameStack.push_back(Ctors);
          }
        }
      }
      if (!Observer.lossy_claiming() ||
          Observer.claimNode(BuildNodeIdForDecl(ID))) {
        return RecursiveASTVisitor::TraverseDecl(ID);
      } else {
        return true;
      }
    }
  }
  if (Decl != nullptr && (!Observer.lossy_claiming() ||
                          Observer.claimNode(BuildNodeIdForDecl(Decl)))) {
    return RecursiveASTVisitor::TraverseDecl(Decl);
  } else {
    return true;
  }
}

bool IndexerASTVisitor::VisitCXXDependentScopeMemberExpr(
    const clang::CXXDependentScopeMemberExpr *E) {
  MaybeFew<GraphObserver::NodeId> Root = None();
  auto BaseType = E->getBaseType();
  if (BaseType.getTypePtrOrNull()) {
    auto *Builtin = dyn_cast<BuiltinType>(BaseType.getTypePtr());
    // The "Dependent" builtin type is not useful, so we'll keep this lookup
    // rootless. We could alternately invent a singleton type for the range of
    // the lhs expression.
    if (!Builtin || Builtin->getKind() != BuiltinType::Dependent) {
      Root = BuildNodeIdForType(BaseType);
    }
  }
  // EmitRanges::Yes causes the use location to be recorded.
  auto DepNodeId =
      BuildNodeIdForDependentName(E->getQualifierLoc(), E->getMember(),
                                  E->getMemberLoc(), Root, EmitRanges::Yes);
  if (DepNodeId && E->hasExplicitTemplateArgs()) {
    std::vector<GraphObserver::NodeId> ArgIds;
    std::vector<const GraphObserver::NodeId *> ArgNodeIds;
    if (BuildTemplateArgumentList(&E->getExplicitTemplateArgs(), nullptr,
                                  ArgIds, ArgNodeIds)) {
      auto TappNodeId =
          Observer.recordTappNode(DepNodeId.primary(), ArgNodeIds);
      auto StmtId = BuildNodeIdForImplicitStmt(E);
      auto Range = clang::SourceRange(E->getMemberLoc(),
                                      E->getLocEnd().getLocWithOffset(1));
      if (auto RCC = RangeInCurrentContext(StmtId, Range)) {
        Observer.recordDeclUseLocation(
            RCC.primary(), TappNodeId,
            GraphObserver::Claimability::Unclaimable);
      }
    }
  }
  return true;
}

bool IndexerASTVisitor::VisitMemberExpr(const clang::MemberExpr *E) {
  if (E->getMemberLoc().isInvalid()) {
    return true;
  }
  if (const auto *FieldDecl = E->getMemberDecl()) {
    auto FieldId = BuildNodeIdForDecl(FieldDecl);
    auto Range = RangeForASTEntityFromSourceLocation(E->getMemberLoc());
    auto StmtId = BuildNodeIdForImplicitStmt(E);
    if (auto RCC = RangeInCurrentContext(StmtId, Range)) {
      Observer.recordDeclUseLocation(RCC.primary(), FieldId,
                                     GraphObserver::Claimability::Unclaimable);
      // TODO(zarko): where do non-explicit template arguments come up?
      if (E->hasExplicitTemplateArgs()) {
        std::vector<GraphObserver::NodeId> ArgIds;
        std::vector<const GraphObserver::NodeId *> ArgNodeIds;
        if (BuildTemplateArgumentList(&E->getExplicitTemplateArgs(), nullptr,
                                      ArgIds, ArgNodeIds)) {
          auto TappNodeId = Observer.recordTappNode(FieldId, ArgNodeIds);
          auto Range = clang::SourceRange(E->getMemberLoc(),
                                          E->getLocEnd().getLocWithOffset(1));
          if (auto RCC = RangeInCurrentContext(StmtId, Range)) {
            Observer.recordDeclUseLocation(
                RCC.primary(), TappNodeId,
                GraphObserver::Claimability::Unclaimable);
          }
        }
      }
    }
  }
  return true;
}

bool IndexerASTVisitor::VisitCXXConstructExpr(
    const clang::CXXConstructExpr *E) {
  if (const auto *Callee = E->getConstructor()) {
    // TODO(zarko): What about static initializers? Do we blame these on the
    // translation unit?
    if (!BlameStack.empty()) {
      clang::SourceLocation RPL = E->getParenOrBraceRange().getEnd();
      clang::SourceRange SR = E->getSourceRange();
      if (RPL.isValid()) {
        // This loses the right paren without the offset.
        SR.setEnd(RPL.getLocWithOffset(1));
      }
      auto StmtId = BuildNodeIdForImplicitStmt(E);
      if (auto RCC = RangeInCurrentContext(StmtId, SR)) {
        RecordCallEdges(RCC.primary(), BuildNodeIdForDecl(Callee));
      }
    }
  }
  return true;
}

bool IndexerASTVisitor::VisitCXXDeleteExpr(const clang::CXXDeleteExpr *E) {
  if (BlameStack.empty()) {
    return true;
  }
  auto DTy = E->getDestroyedType();
  if (DTy.isNull()) {
    return true;
  }
  auto DTyNonRef = DTy.getNonReferenceType();
  if (DTyNonRef.isNull()) {
    return true;
  }
  auto BaseType = Context.getBaseElementType(DTyNonRef);
  if (BaseType.isNull()) {
    return true;
  }
  MaybeFew<GraphObserver::NodeId> DDId;
  if (const auto *CD = BaseType->getAsCXXRecordDecl()) {
    if (const auto *DD = CD->getDestructor()) {
      DDId = BuildNodeIdForDecl(DD);
    }
  } else {
    auto QTCan = BaseType.getCanonicalType();
    if (QTCan.isNull()) {
      return true;
    }
    auto TyId = BuildNodeIdForType(QTCan);
    if (!TyId) {
      return true;
    }
    auto DtorName = Context.DeclarationNames.getCXXDestructorName(
        CanQualType::CreateUnsafe(QTCan));
    DDId =
        BuildNodeIdForDependentName(clang::NestedNameSpecifierLoc(), DtorName,
                                    E->getLocStart(), TyId, EmitRanges::No);
  }
  if (DDId) {
    clang::SourceRange SR = E->getSourceRange();
    SR.setEnd(SR.getEnd().getLocWithOffset(1));
    auto StmtId = BuildNodeIdForImplicitStmt(E);
    if (auto RCC = RangeInCurrentContext(StmtId, SR)) {
      RecordCallEdges(RCC.primary(), DDId.primary());
    }
  }
  return true;
}

bool IndexerASTVisitor::VisitCXXPseudoDestructorExpr(
    const clang::CXXPseudoDestructorExpr *E) {
  if (E->getDestroyedType().isNull()) {
    return true;
  }
  auto DTCan = E->getDestroyedType().getCanonicalType();
  if (DTCan.isNull() || !DTCan.isCanonical()) {
    return true;
  }
  MaybeFew<GraphObserver::NodeId> TyId;
  clang::NestedNameSpecifierLoc NNSLoc;
  if (E->getDestroyedTypeInfo() != nullptr) {
    TyId = BuildNodeIdForType(E->getDestroyedTypeInfo()->getTypeLoc(),
                              E->getDestroyedTypeInfo()->getTypeLoc().getType(),
                              EmitRanges::Yes);
  } else if (E->hasQualifier()) {
    NNSLoc = E->getQualifierLoc();
  }
  auto DtorName = Context.DeclarationNames.getCXXDestructorName(
      CanQualType::CreateUnsafe(DTCan));
  if (auto DDId = BuildNodeIdForDependentName(
          NNSLoc, DtorName, E->getTildeLoc(), TyId, EmitRanges::Yes)) {
    clang::SourceRange SR = E->getSourceRange();
    SR.setEnd(RangeForASTEntityFromSourceLocation(SR.getEnd()).getEnd());
    auto StmtId = BuildNodeIdForImplicitStmt(E);
    if (auto RCC = RangeInCurrentContext(StmtId, SR)) {
      RecordCallEdges(RCC.primary(), DDId.primary());
    }
  }
  return true;
}

bool IndexerASTVisitor::VisitCXXUnresolvedConstructExpr(
    const clang::CXXUnresolvedConstructExpr *E) {
  if (!BlameStack.empty()) {
    auto QTCan = E->getTypeAsWritten().getCanonicalType();
    if (QTCan.isNull()) {
      return true;
    }
    CHECK(E->getTypeSourceInfo() != nullptr);
    auto TyId = BuildNodeIdForType(
        E->getTypeSourceInfo()->getTypeLoc(),
        E->getTypeSourceInfo()->getTypeLoc().getType(), EmitRanges::Yes);
    if (!TyId) {
      return true;
    }
    auto CtorName = Context.DeclarationNames.getCXXConstructorName(
        CanQualType::CreateUnsafe(QTCan));
    if (auto LookupId = BuildNodeIdForDependentName(
            clang::NestedNameSpecifierLoc(), CtorName, E->getLocStart(), TyId,
            EmitRanges::No)) {
      clang::SourceLocation RPL = E->getRParenLoc();
      clang::SourceRange SR = E->getSourceRange();
      // This loses the right paren without the offset.
      if (RPL.isValid()) {
        SR.setEnd(RPL.getLocWithOffset(1));
      }
      auto StmtId = BuildNodeIdForImplicitStmt(E);
      if (auto RCC = RangeInCurrentContext(StmtId, SR)) {
        RecordCallEdges(RCC.primary(), LookupId.primary());
      }
    }
  }
  return true;
}

bool IndexerASTVisitor::VisitCallExpr(const clang::CallExpr *E) {
  if (BlameStack.empty()) {
    // TODO(zarko): What about static initializers? Do we blame these on the
    // translation unit?
    return true;
  }
  clang::SourceLocation RPL = E->getRParenLoc();
  clang::SourceRange SR = E->getSourceRange();
  if (RPL.isValid()) {
    // This loses the right paren without the offset.
    SR.setEnd(RPL.getLocWithOffset(1));
  }
  auto StmtId = BuildNodeIdForImplicitStmt(E);
  if (auto RCC = RangeInCurrentContext(StmtId, SR)) {
    if (const auto *Callee = E->getCalleeDecl()) {
      RecordCallEdges(RCC.primary(), BuildNodeIdForDecl(Callee));
    } else if (const auto *CE = E->getCallee()) {
      if (auto CalleeId = BuildNodeIdForExpr(CE, EmitRanges::Yes)) {
        RecordCallEdges(RCC.primary(), CalleeId.primary());
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

bool IndexerASTVisitor::VisitUnaryExprOrTypeTraitExpr(
    const clang::UnaryExprOrTypeTraitExpr *E) {
  if (E->isArgumentType()) {
    auto *TSI = E->getArgumentTypeInfo();
    if (!TSI->getTypeLoc().isNull()) {
      // TODO(zarko): Possibly discern the Decl for TargetType and call
      // InspectDeclRef on it.
      BuildNodeIdForType(TSI->getTypeLoc(), TSI->getType(), EmitRanges::Yes);
    }
  }
  return true;
}

bool IndexerASTVisitor::VisitDeclRefExpr(const clang::DeclRefExpr *DRE) {
  // TODO(zarko): check to see if this DeclRefExpr has already been indexed.
  // (Use a simple N=1 cache.)
  // Use FoundDecl to get to template defs; use getDecl to get to template
  // instantiations.
  const NamedDecl *const FoundDecl = DRE->getDecl();
  // TODO(zarko): Point at the capture as well as the thing being captured;
  // port over RemapDeclIfCaptured.
  // const NamedDecl* const TargetDecl = RemapDeclIfCaptured(FoundDecl);
  const NamedDecl *TargetDecl = FoundDecl;
  if (const auto *IFD = dyn_cast<clang::IndirectFieldDecl>(FoundDecl)) {
    // An IndirectFieldDecl is just an alias; we want to record this as a
    // reference to the underlying entity.
    // TODO(jdennett): Would this be better done in BuildNodeIdForDecl?
    TargetDecl = IFD->getAnonField();
  }
  if (isa<clang::VarDecl>(TargetDecl) && TargetDecl->isImplicit()) {
    // Ignore variable declarations synthesized from for-range loops, as they
    // are just a clang implementation detail.
    return true;
  }
  SourceLocation SL = DRE->getLocation();
  if (SL.isValid()) {
    SourceRange Range = RangeForASTEntityFromSourceLocation(SL);
    auto StmtId = BuildNodeIdForImplicitStmt(DRE);
    if (auto RCC = RangeInCurrentContext(StmtId, Range)) {
      GraphObserver::NodeId DeclId = BuildNodeIdForDecl(TargetDecl);
      Observer.recordDeclUseLocation(RCC.primary(), DeclId,
                                     GraphObserver::Claimability::Unclaimable);
      for (const auto &S : Supports) {
        S->InspectDeclRef(*this, SL, RCC.primary(), DeclId, TargetDecl);
      }
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
  if (Decl->isImplicit()) {
    // Ignore variable declarations synthesized from for-range loops, as they
    // are just a clang implementation detail.
    return true;
  }
  SourceLocation DeclLoc = Decl->getLocation();
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  GraphObserver::NodeId BodyDeclNode(Observer.getDefaultClaimToken(), "");
  GraphObserver::NodeId DeclNode(Observer.getDefaultClaimToken(), "");
  const clang::ASTTemplateArgumentListInfo *ArgsAsWritten = nullptr;
  if (const auto *VTPSD =
          dyn_cast<const clang::VarTemplatePartialSpecializationDecl>(Decl)) {
    ArgsAsWritten = VTPSD->getTemplateArgsAsWritten();
    BodyDeclNode = BuildNodeIdForDecl(Decl, 0);
    DeclNode = RecordTemplate(VTPSD, BodyDeclNode);
  } else if (const auto *VTD = Decl->getDescribedVarTemplate()) {
    CHECK(!isa<clang::VarTemplateSpecializationDecl>(VTD));
    BodyDeclNode = BuildNodeIdForDecl(Decl);
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
              Observer.recordTappNode(SpecializedNode.primary(), NIDPS),
              GraphObserver::Confidence::NonSpeculative);
        }
        Observer.recordSpecEdge(
            DeclNode, Observer.recordTappNode(SpecializedNode.primary(), NIDPS),
            GraphObserver::Confidence::NonSpeculative);
      }
    }
    if (const auto *Partial =
            PrimaryOrPartial
                .dyn_cast<clang::VarTemplatePartialSpecializationDecl *>()) {
      if (BuildTemplateArgumentList(
              nullptr, &VTSD->getTemplateInstantiationArgs(), NIDS, NIDPS)) {
        Observer.recordInstEdge(
            DeclNode,
            Observer.recordTappNode(BuildNodeIdForDecl(Partial), NIDPS),
            GraphObserver::Confidence::NonSpeculative);
      }
    }
  }

  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode);
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
    Observer.recordVariableNode(
        VarNameId, BodyDeclNode, GraphObserver::Completeness::Incomplete,
        GraphObserver::VariableSubkind::None, GetFormat(Decl));
    for (const auto &S : Supports) {
      S->InspectVariable(*this, DeclNode, BodyDeclNode, Decl,
                         GraphObserver::Completeness::Incomplete, Completions);
    }
    return true;
  }
  FileID DeclFile = Observer.getSourceManager()->getFileID(Decl->getLocation());
  auto NameRangeInContext =
      RangeInCurrentContext(Decl->isImplicit(), BodyDeclNode, NameRange);
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
      if (NameRangeInContext) {
        Observer.recordCompletionRange(
            NameRangeInContext.primary(), TargetDecl,
            NextDeclFile == DeclFile
                ? GraphObserver::Specificity::UniquelyCompletes
                : GraphObserver::Specificity::Completes);
      }
      Completions.push_back(LibrarySupport::Completion{NextDecl, TargetDecl});
    }
  }
  Observer.recordVariableNode(
      VarNameId, BodyDeclNode, GraphObserver::Completeness::Definition,
      GraphObserver::VariableSubkind::None, GetFormat(Decl));
  for (const auto &S : Supports) {
    S->InspectVariable(*this, DeclNode, BodyDeclNode, Decl,
                       GraphObserver::Completeness::Definition, Completions);
  }
  return true;
}

bool IndexerASTVisitor::VisitNamespaceDecl(const clang::NamespaceDecl *Decl) {
  GraphObserver::NameId DeclName(BuildNameIdForDecl(Decl));
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  // Use the range covering `namespace` for anonymous namespaces.
  SourceRange NameRange;
  if (Decl->isAnonymousNamespace()) {
    SourceLocation Loc = Decl->getLocStart();
    if (Decl->isInline() && Loc.isValid() && Loc.isFileID()) {
      // Skip the `inline` keyword.
      Loc = RangeForSingleTokenFromSourceLocation(Loc).getEnd();
      if (Loc.isValid() && Loc.isFileID()) {
        SkipWhitespace(*Observer.getSourceManager(), &Loc);
      }
    }
    if (Loc.isValid() && Loc.isFileID()) {
      NameRange = RangeForASTEntityFromSourceLocation(Loc);
    }
  } else {
    NameRange = RangeForNameOfDeclaration(Decl);
  }
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode);
  Observer.recordNamespaceNode(DeclName, DeclNode, GetFormat(Decl));
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  return true;
}

bool IndexerASTVisitor::VisitFieldDecl(const clang::FieldDecl *Decl) {
  GraphObserver::NameId DeclName(BuildNameIdForDecl(Decl));
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  SourceLocation DeclLoc = Decl->getLocation();
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode);
  // TODO(zarko): Record completeness data. This is relevant for static fields,
  // which may be declared along with a complete class definition but later
  // defined in a separate translation unit.
  Observer.recordVariableNode(
      DeclName, DeclNode, GraphObserver::Completeness::Definition,
      GraphObserver::VariableSubkind::Field, GetFormat(Decl));
  if (const auto *TSI = Decl->getTypeSourceInfo()) {
    // TODO(zarko): Record storage classes for fields.
    AscribeSpelledType(TSI->getTypeLoc(), Decl->getType(), DeclNode);
  } else if (auto TyNodeId = BuildNodeIdForType(Decl->getType())) {
    Observer.recordTypeEdge(DeclNode, TyNodeId.primary());
  }
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  return true;
}

bool IndexerASTVisitor::VisitEnumConstantDecl(
    const clang::EnumConstantDecl *Decl) {
  // We first build the NameId and NodeId for the enumerator.
  GraphObserver::NameId DeclName(BuildNameIdForDecl(Decl));
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  SourceLocation DeclLoc = Decl->getLocation();
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode);
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
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode);
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
      if (auto RCC =
              RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange)) {
        Observer.recordCompletionRange(
            RCC.primary(), BuildNodeIdForDecl(NextDecl),
            NextDeclFile == DeclFile
                ? GraphObserver::Specificity::UniquelyCompletes
                : GraphObserver::Specificity::Completes);
      }
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
  if (!TraverseDecl(TD->getTemplatedDecl())) {
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

bool IndexerASTVisitor::TraverseTypeAliasTemplateDecl(
    clang::TypeAliasTemplateDecl *TATD) {
  TypeContext.push_back(TATD->getTemplateParameters());
  TraverseDecl(TATD->getTemplatedDecl());
  TypeContext.pop_back();
  return true;
}

bool IndexerASTVisitor::TraverseFunctionTemplateDecl(
    clang::FunctionTemplateDecl *FTD) {
  TypeContext.push_back(FTD->getTemplateParameters());
  // We traverse the template parameter list when we visit the FunctionDecl.
  TraverseDecl(FTD->getTemplatedDecl());
  TypeContext.pop_back();
  // See also RecursiveAstVisitor<T>::TraverseTemplateInstantiations.
  if (FTD == FTD->getCanonicalDecl()) {
    for (auto *FD : FTD->specializations()) {
      for (auto *RD : FD->redecls()) {
        if (RD->getTemplateSpecializationKind() !=
            clang::TSK_ExplicitSpecialization) {
          TraverseDecl(RD);
        }
      }
    }
  }
  return true;
}

MaybeFew<GraphObserver::Range>
IndexerASTVisitor::ExplicitRangeInCurrentContext(const clang::SourceRange &SR) {
  if (!SR.getBegin().isValid()) {
    return None();
  }
  if (!RangeContext.empty()) {
    return GraphObserver::Range(SR, RangeContext.back());
  } else {
    return GraphObserver::Range(SR, Observer.getClaimTokenForRange(SR));
  }
}

MaybeFew<GraphObserver::Range>
IndexerASTVisitor::RangeInCurrentContext(bool implicit,
                                         const GraphObserver::NodeId &Id,
                                         const clang::SourceRange &SR) {
  return implicit ? GraphObserver::Range(Id)
                  : ExplicitRangeInCurrentContext(SR);
}

template <typename TemplateDeclish>
GraphObserver::NodeId
IndexerASTVisitor::RecordTemplate(const TemplateDeclish *Decl,
                                  const GraphObserver::NodeId &BodyDeclNode) {
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  Observer.recordChildOfEdge(BodyDeclNode, DeclNode);
  Observer.recordAbsNode(DeclNode);
  for (const auto *ND : *Decl->getTemplateParameters()) {
    GraphObserver::NodeId ParamId(Observer.getDefaultClaimToken(), "");
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
      LOG(FATAL) << "Unknown entry in TemplateParameterList";
    }
    SourceRange Range = RangeForNameOfDeclaration(ND);
    MaybeRecordDefinitionRange(
        RangeInCurrentContext(Decl->isImplicit(), ParamId, Range), ParamId);
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
  GraphObserver::NodeId BodyDeclNode(Observer.getDefaultClaimToken(), "");
  GraphObserver::NodeId DeclNode(Observer.getDefaultClaimToken(), "");
  const clang::ASTTemplateArgumentListInfo *ArgsAsWritten = nullptr;
  if (const auto *CTPSD =
          dyn_cast<const clang::ClassTemplatePartialSpecializationDecl>(Decl)) {
    ArgsAsWritten = CTPSD->getTemplateArgsAsWritten();
    BodyDeclNode = BuildNodeIdForDecl(Decl, 0);
    DeclNode = RecordTemplate(CTPSD, BodyDeclNode);
  } else if (auto *CRD = dyn_cast<const clang::CXXRecordDecl>(Decl)) {
    if (const auto *CTD = CRD->getDescribedClassTemplate()) {
      CHECK(!isa<clang::ClassTemplateSpecializationDecl>(CRD));
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
    std::vector<GraphObserver::NodeId> NIDS;
    std::vector<const GraphObserver::NodeId *> NIDPS;
    auto PrimaryOrPartial = CTSD->getSpecializedTemplateOrPartial();
    if (BuildTemplateArgumentList(ArgsAsWritten, &CTSD->getTemplateArgs(), NIDS,
                                  NIDPS)) {
      if (auto SpecializedNode = BuildNodeIdForTemplateName(
              clang::TemplateName(CTSD->getSpecializedTemplate()),
              CTSD->getPointOfInstantiation())) {
        if (PrimaryOrPartial.is<clang::ClassTemplateDecl *>() &&
            !isa<const clang::ClassTemplatePartialSpecializationDecl>(Decl)) {
          // This is both an instance and a specialization of the primary
          // template. We use the same arguments list for both.
          Observer.recordInstEdge(
              DeclNode,
              Observer.recordTappNode(SpecializedNode.primary(), NIDPS),
              GraphObserver::Confidence::NonSpeculative);
        }
        Observer.recordSpecEdge(
            DeclNode, Observer.recordTappNode(SpecializedNode.primary(), NIDPS),
            GraphObserver::Confidence::NonSpeculative);
      }
    }
    if (const auto *Partial =
            PrimaryOrPartial
                .dyn_cast<clang::ClassTemplatePartialSpecializationDecl *>()) {
      if (BuildTemplateArgumentList(
              nullptr, &CTSD->getTemplateInstantiationArgs(), NIDS, NIDPS)) {
        Observer.recordInstEdge(
            DeclNode,
            Observer.recordTappNode(BuildNodeIdForDecl(Partial), NIDPS),
            GraphObserver::Confidence::NonSpeculative);
      }
    }
  }
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode);
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
                              GraphObserver::Completeness::Incomplete,
                              GetFormat(Decl));
    return true;
  }
  FileID DeclFile = Observer.getSourceManager()->getFileID(Decl->getLocation());
  if (auto NameRangeInContext =
          RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange)) {
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
        // We should not point a completes edge from an abs node to a record
        // node.
        GraphObserver::NodeId TargetDecl =
            BuildNodeIdForDecl(OuterTemplate ? OuterTemplate : NextDecl);
        Observer.recordCompletionRange(
            NameRangeInContext.primary(), TargetDecl,
            NextDeclFile == DeclFile
                ? GraphObserver::Specificity::UniquelyCompletes
                : GraphObserver::Specificity::Completes);
      }
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
                            GraphObserver::Completeness::Definition,
                            GetFormat(Decl));
  return true;
}

bool IndexerASTVisitor::VisitFunctionDecl(clang::FunctionDecl *Decl) {
  GraphObserver::NodeId InnerNode(Observer.getDefaultClaimToken(), "");
  GraphObserver::NodeId OuterNode(Observer.getDefaultClaimToken(), "");
  // There are five flavors of function (see TemplateOrSpecialization in
  // FunctionDecl).
  const clang::TemplateArgumentLoc *ArgsAsWritten = nullptr;
  unsigned NumArgsAsWritten = 0;
  const clang::TemplateArgumentList *Args = nullptr;
  std::vector<std::pair<clang::TemplateName, clang::SourceLocation>> TNs;
  bool TNsAreSpeculative = false;
  if (auto *FTD = Decl->getDescribedFunctionTemplate()) {
    // Function template (inc. overloads)
    InnerNode = BuildNodeIdForDecl(Decl, 0);
    OuterNode = RecordTemplate(FTD, InnerNode);
  } else if (auto *MSI = Decl->getMemberSpecializationInfo()) {
    // This case is uninteresting: any template variables are bound
    // by our enclosing context.
    // For example:
    // `template <typename T> struct S { int f() { return 0; } };`
    // `int q() { S<int> s; return s.f(); }`
    InnerNode = BuildNodeIdForDecl(Decl);
    OuterNode = InnerNode;
  } else if (auto *FTSI = Decl->getTemplateSpecializationInfo()) {
    if (FTSI->TemplateArgumentsAsWritten) {
      ArgsAsWritten = FTSI->TemplateArgumentsAsWritten->getTemplateArgs();
      NumArgsAsWritten = FTSI->TemplateArgumentsAsWritten->NumTemplateArgs;
    }
    Args = FTSI->TemplateArguments;
    TNs.emplace_back(TemplateName(FTSI->getTemplate()),
                     FTSI->getPointOfInstantiation());
    InnerNode = BuildNodeIdForDecl(Decl);
    OuterNode = InnerNode;
  } else if (auto *DFTSI = Decl->getDependentSpecializationInfo()) {
    // From Clang's documentation:
    // "Since explicit function template specialization and instantiation
    // declarations can only appear in namespace scope, and you can only
    // specialize a member of a fully-specialized class, the only way to get
    // one of these is in a friend declaration like the following:
    //   template <typename T> void f(T t);
    //   template <typename T> struct S {
    //     friend void f<>(T t);
    //   };
    TNsAreSpeculative = true;
    // There doesn't appear to be an equivalent operation to
    // VarTemplateSpecializationDecl::getTemplateInstantiationArgs (and
    // it's unclear whether one should even be defined). This means that we'll
    // only be able to record template arguments that were written down in the
    // file; for instance, in the example above, we'll indicate that f may
    // specialize primary template f applied to no arguments. If instead the
    // code read `friend void f<T>(T t)`, we would record that it specializes
    // the primary template with type variable T.
    ArgsAsWritten = DFTSI->getTemplateArgs();
    NumArgsAsWritten = DFTSI->getNumTemplateArgs();
    for (unsigned T = 0; T < DFTSI->getNumTemplates(); ++T) {
      TNs.emplace_back(clang::TemplateName(DFTSI->getTemplate(T)),
                       Decl->getLocation());
    }
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
      NIDS.reserve(NumArgsAsWritten);
      NIDPS.reserve(NumArgsAsWritten);
      for (unsigned I = 0; I < NumArgsAsWritten; ++I) {
        if (auto ArgId = BuildNodeIdForTemplateArgument(ArgsAsWritten[I],
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
      for (const auto &NID : NIDS) {
        NIDPS.push_back(&NID);
      }
      auto Confidence = TNsAreSpeculative
                            ? GraphObserver::Confidence::Speculative
                            : GraphObserver::Confidence::NonSpeculative;
      for (const auto &TN : TNs) {
        if (auto SpecializedNode =
                BuildNodeIdForTemplateName(TN.first, TN.second)) {
          // Because partial specialization of function templates is forbidden,
          // instantiates edges will always choose the same type (a tapp with
          // the primary template as its first argument) as specializes edges.
          Observer.recordInstEdge(
              OuterNode,
              Observer.recordTappNode(SpecializedNode.primary(), NIDPS),
              Confidence);
          Observer.recordSpecEdge(
              OuterNode,
              Observer.recordTappNode(SpecializedNode.primary(), NIDPS),
              Confidence);
        }
      }
    }
  }
  GraphObserver::NameId DeclName(BuildNameIdForDecl(Decl));
  SourceLocation DeclLoc = Decl->getLocation();
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  auto NameRangeInContext(
      RangeInCurrentContext(Decl->isImplicit(), OuterNode, NameRange));
  MaybeRecordDefinitionRange(NameRangeInContext, OuterNode);
  Observer.recordNamedEdge(OuterNode, DeclName);
  bool IsFunctionDefinition = IsDefinition(Decl);
  unsigned ParamNumber = 0;
  for (const auto *Param : Decl->params()) {
    GraphObserver::NodeId VarNodeId(BuildNodeIdForDecl(Param));
    GraphObserver::NameId VarNameId(BuildNameIdForDecl(Param));
    SourceRange Range = RangeForNameOfDeclaration(Param);
    Observer.recordVariableNode(
        VarNameId, VarNodeId,
        IsFunctionDefinition ? GraphObserver::Completeness::Definition
                             : GraphObserver::Completeness::Incomplete,
        GraphObserver::VariableSubkind::None, GetFormat(Param));
    MaybeRecordDefinitionRange(
        RangeInCurrentContext(Param->isImplicit() || Decl->isImplicit(),
                              VarNodeId, Range),
        VarNodeId);
    Observer.recordParamEdge(InnerNode, ParamNumber++, VarNodeId);
    MaybeFew<GraphObserver::NodeId> ParamType;
    if (auto *TSI = Param->getTypeSourceInfo()) {
      ParamType = BuildNodeIdForType(TSI->getTypeLoc(), EmitRanges::No);
    } else {
      CHECK(!Param->getType().isNull());
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

  if (const auto *MF = dyn_cast<CXXMethodDecl>(Decl)) {
    const auto *R = MF->getParent();
    GraphObserver::NodeId ParentNode(BuildNodeIdForDecl(R));
    // OO_Call, OO_Subscript, and OO_Equal must be member functions.
    // The dyn_cast to CXXMethodDecl above is therefore not dropping
    // (impossible) free function incarnations of these operators from
    // consideration in the following.
    for (auto O = MF->begin_overridden_methods(),
              E = MF->end_overridden_methods();
         O != E; ++O) {
      Observer.recordOverridesEdge(InnerNode, BuildNodeIdForDecl(*O));
    }
  }

  AddChildOfEdgeToDeclContext(Decl, OuterNode);
  GraphObserver::FunctionSubkind Subkind = GraphObserver::FunctionSubkind::None;
  if (const auto *CC = dyn_cast<CXXConstructorDecl>(Decl)) {
    Subkind = GraphObserver::FunctionSubkind::Constructor;
    size_t InitNumber = 0;
    for (const auto *Init : CC->inits()) {
      ++InitNumber;
      if (const auto *InitTy = Init->getTypeSourceInfo()) {
        auto QT = InitTy->getType().getCanonicalType();
        if (QT.getTypePtr()->isDependentType()) {
          if (auto TyId = BuildNodeIdForType(InitTy->getTypeLoc(), QT,
                                             EmitRanges::Yes)) {
            auto DepName = Context.DeclarationNames.getCXXConstructorName(
                CanQualType::CreateUnsafe(QT));
            if (auto LookupId = BuildNodeIdForDependentName(
                    clang::NestedNameSpecifierLoc(), DepName,
                    Init->getSourceLocation(), TyId, EmitRanges::No)) {
              clang::SourceLocation RPL = Init->getRParenLoc();
              clang::SourceRange SR = Init->getSourceRange();
              SR.setEnd(SR.getEnd().getLocWithOffset(1));
              if (Init->isWritten()) {
                if (auto RCC = ExplicitRangeInCurrentContext(SR)) {
                  RecordCallEdges(RCC.primary(), LookupId.primary());
                }
              } else {
                // clang::CXXCtorInitializer is its own special flavor of AST
                // node that needs extra care.
                auto InitIdent = LookupId.primary().getRawIdentity() +
                                 std::to_string(InitNumber);
                auto InitId = GraphObserver::NodeId(
                    LookupId.primary().getToken(), InitIdent);
                RecordCallEdges(GraphObserver::Range(InitId),
                                LookupId.primary());
              }
            }
          }
        }
      }
    }
  } else if (const auto *CD = dyn_cast<CXXDestructorDecl>(Decl)) {
    Subkind = GraphObserver::FunctionSubkind::Destructor;
  }
  if (!IsFunctionDefinition) {
    Observer.recordFunctionNode(InnerNode,
                                GraphObserver::Completeness::Incomplete,
                                Subkind, GetFormat(Decl));
    return true;
  }
  if (NameRangeInContext) {
    FileID DeclFile =
        Observer.getSourceManager()->getFileID(Decl->getLocation());
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
            NameRangeInContext.primary(), TargetDecl,
            NextDeclFile == DeclFile
                ? GraphObserver::Specificity::UniquelyCompletes
                : GraphObserver::Specificity::Completes);
      }
    }
  }
  Observer.recordFunctionNode(InnerNode,
                              GraphObserver::Completeness::Definition, Subkind,
                              GetFormat(Decl));
  return true;
}

MaybeFew<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForTypedefNameDecl(
    const clang::TypedefNameDecl *Decl) {
  const auto Cached = DeclToNodeId.find(Decl);
  if (Cached != DeclToNodeId.end()) {
    return Cached->second;
  }

  clang::TypeSourceInfo *TSI = Decl->getTypeSourceInfo();
  if (auto AliasedTypeId =
          BuildNodeIdForType(TSI->getTypeLoc(), EmitRanges::Yes)) {
    GraphObserver::NameId AliasNameId(BuildNameIdForDecl(Decl));
    return Observer.recordTypeAliasNode(AliasNameId, AliasedTypeId.primary(),
                                        GetFormat(Decl));
  }
  return None();
}

bool IndexerASTVisitor::VisitTypedefNameDecl(
    const clang::TypedefNameDecl *Decl) {
  if (Decl == Context.getBuiltinVaListDecl() ||
      Decl == Context.getInt128Decl() || Decl == Context.getUInt128Decl()) {
    // Don't index __uint128_t, __builtin_va_list, __int128_t
    return true;
  }
  if (auto InnerNodeId = BuildNodeIdForTypedefNameDecl(Decl)) {
    GraphObserver::NodeId OuterNodeId = InnerNodeId.primary();
    // If this is a template, we need to emit an abs node for it.
    if (auto *TA = dyn_cast_or_null<TypeAliasDecl>(Decl)) {
      if (auto *TATD = TA->getDescribedAliasTemplate()) {
        OuterNodeId = RecordTemplate(TATD, InnerNodeId.primary());
      }
    }
    SourceRange Range = RangeForNameOfDeclaration(Decl);
    MaybeRecordDefinitionRange(
        RangeInCurrentContext(Decl->isImplicit(), OuterNodeId, Range),
        OuterNodeId);
    AddChildOfEdgeToDeclContext(Decl, OuterNodeId);
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
  CHECK(D != nullptr);
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

// TODO(zarko): Can this logic be shared with BuildNameIdForDecl?
MaybeFew<GraphObserver::NodeId>
IndexerASTVisitor::GetParentForFormat(const clang::Decl *Decl) {
  clang::ast_type_traits::DynTypedNode CurrentNode =
      clang::ast_type_traits::DynTypedNode::create(*Decl);
  const clang::Decl *CurrentNodeAsDecl;
  while (!(CurrentNodeAsDecl = CurrentNode.get<clang::Decl>()) ||
         !isa<clang::TranslationUnitDecl>(CurrentNodeAsDecl)) {
    IndexedParentVector IPV = getIndexedParents(CurrentNode);
    if (IPV.empty()) {
      break;
    }
    // Pick the first path we took to get to this node.
    IndexedParent IP = IPV[0];
    // We would rather name 'template <etc> class C' as C, not C::C, but
    // we also want to be able to give useful names to templates when they're
    // explicitly requested. Therefore:
    if (CurrentNodeAsDecl == Decl ||
        (CurrentNodeAsDecl && isa<ClassTemplateDecl>(CurrentNodeAsDecl))) {
      CurrentNode = IP.Parent;
      continue;
    }
    if (CurrentNodeAsDecl) {
      if (const NamedDecl *ND = dyn_cast<NamedDecl>(CurrentNodeAsDecl)) {
        return BuildNodeIdForDecl(ND);
      }
    }
    CurrentNode = IP.Parent;
    if (CurrentNodeAsDecl) {
      if (const auto *DC = CurrentNodeAsDecl->getDeclContext()) {
        if (const TagDecl *TD = dyn_cast<TagDecl>(CurrentNodeAsDecl)) {
          const clang::Decl *DCD;
          switch (DC->getDeclKind()) {
          case Decl::Namespace:
            DCD = dyn_cast<NamespaceDecl>(DC);
            break;
          case Decl::TranslationUnit:
            DCD = dyn_cast<TranslationUnitDecl>(DC);
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
  return None();
}

bool IndexerASTVisitor::AddFormatToStream(llvm::raw_string_ostream &Ostream,
                                          const clang::NamedDecl *ND) {
  auto Name = ND->getDeclName();
  auto II = Name.getAsIdentifierInfo();
  if (II && !II->getName().empty()) {
    Ostream << EscapeForFormatLiteral(II->getName());
    return true;
  } else if (Name.getCXXOverloadedOperator() != OO_None) {
    switch (Name.getCXXOverloadedOperator()) {
#define OVERLOADED_OPERATOR(Name, Spelling, Token, Unary, Binary, MemberOnly)  \
  case OO_##Name:                                                              \
    Ostream << EscapeForFormatLiteral("operator " #Name);
#include "clang/Basic/OperatorKinds.def"
#undef OVERLOADED_OPERATOR
      return true;
    default:
      break;
    }
  } else if (const auto *MD = dyn_cast<clang::CXXMethodDecl>(ND)) {
    if (isa<clang::CXXConstructorDecl>(MD)) {
      Ostream << EscapeForFormatLiteral("(ctor)");
      return true;
    } else if (isa<clang::CXXDestructorDecl>(MD)) {
      Ostream << EscapeForFormatLiteral("(dtor)");
      return true;
    } else if (const auto *CD = dyn_cast<clang::CXXConversionDecl>(MD)) {
      auto ToType = CD->getConversionType();
      if (!ToType.isNull()) {
        std::string Substring;
        llvm::raw_string_ostream Substream(Substring);
        Substream << "operator ";
        ToType.print(Substream,
                     clang::PrintingPolicy(*Observer.getLangOptions()));
        Substream.flush();
        Ostream << EscapeForFormatLiteral(Substring);
        return true;
      }
    }
  }
  return false;
}

std::string IndexerASTVisitor::GetFormat(const clang::NamedDecl *Decl) {
  std::string Str;
  llvm::raw_string_ostream Ostream(Str);
  Ostream << "%^::";
  if (!AddFormatToStream(Ostream, Decl)) {
    Ostream << EscapeForFormatLiteral("(anonymous)");
  }
  Ostream.flush();
  return Str;
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
      // This is an anonymous namespace. We have two cases:
      // If there is any file that is not a textual include (.inc,
      //     or explicitly marked as such in a module) between the declaration
      //     site and the main source file, then the namespace's identifier is
      //     the shared anonymous namespace identifier "@#anon"
      // Otherwise, it's the anonymous namespace identifier associated with the
      //     main source file.
      if (Observer.isMainSourceFileRelatedLocation(ND->getLocation())) {
        Observer.AppendMainSourceFileIdentifierToStream(Ostream);
      }
      Ostream << "@#anon";
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
    } else if (const auto *MD = dyn_cast<clang::CXXMethodDecl>(ND)) {
      if (isa<clang::CXXConstructorDecl>(MD)) {
        return AddNameToStream(Ostream, MD->getParent());
      } else if (isa<clang::CXXDestructorDecl>(MD)) {
        Ostream << "~";
        // Append the parent name or a dependent index if the parent is
        // nameless.
        return AddNameToStream(Ostream, MD->getParent());
      } else if (const auto *CD = dyn_cast<clang::CXXConversionDecl>(MD)) {
        auto ToType = CD->getConversionType();
        if (ToType.isNull()) {
          return false;
        }
        ToType.print(Ostream,
                     clang::PrintingPolicy(*Observer.getLangOptions()));
        return true;
      }
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
  if (!Verbosity && !const_cast<clang::Decl *>(Decl)->isLocalExternDecl() &&
      Decl->getParentFunctionOrMethod() != nullptr) {
    Id.Hidden = true;
  }
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
          switch (DC->getDeclKind()) {
          case Decl::Namespace:
            DCD = dyn_cast<NamespaceDecl>(DC);
            break;
          case Decl::TranslationUnit:
            DCD = dyn_cast<TranslationUnitDecl>(DC);
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
    CHECK(IgnoreUnimplemented) << "SemanticHash(OverloadedTemplate)";
    return 0;
  case TemplateName::QualifiedTemplate:
    CHECK(IgnoreUnimplemented) << "SemanticHash(QualifiedTemplate)";
    return 0;
  case TemplateName::DependentTemplate:
    CHECK(IgnoreUnimplemented) << "SemanticHash(DependentTemplate)";
    return 0;
  case TemplateName::SubstTemplateTemplateParm:
    CHECK(IgnoreUnimplemented) << "SemanticHash(SubstTemplateTemplateParm)";
    return 0;
  case TemplateName::SubstTemplateTemplateParmPack:
    CHECK(IgnoreUnimplemented) << "SemanticHash(SubstTemplateTemplateParmPack)";
    return 0;
  default:
    LOG(FATAL) << "Unexpected TemplateName Kind";
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
    CHECK(IgnoreUnimplemented) << "SemanticHash(Declaration)";
    return 0;
  case TemplateArgument::NullPtr:
    CHECK(IgnoreUnimplemented) << "SemanticHash(NullPtr)";
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
    CHECK(IgnoreUnimplemented) << "SemanticHash(TemplateExpansion)";
    return 0;
  case TemplateArgument::Expression:
    CHECK(IgnoreUnimplemented) << "SemanticHash(Expression)";
    return 0;
  case TemplateArgument::Pack: {
    uint64_t out = 0x8080808008080808LL;
    for (const auto &Element : TA.pack_elements()) {
      out = SemanticHash(Element) ^ ((out << 1) | (out >> 63));
    }
    return out;
  }
  default:
    LOG(FATAL) << "Unexpected TemplateArgument Kind";
  }
  return 0;
}

uint64_t IndexerASTVisitor::SemanticHash(const clang::QualType &T) {
  QualType CQT(T.getCanonicalType());
  return std::hash<std::string>()(CQT.getAsString());
}

uint64_t IndexerASTVisitor::SemanticHash(const clang::EnumDecl *ED) {
  // Memoize semantic hashes for enums, as they are also needed to create
  // names for enum constants.
  auto Ins = EnumToHash.insert(std::make_pair(ED, 0));
  if (!Ins.second)
    return Ins.first->second;

  // TODO(zarko): Do we need a better hash function?
  uint64_t hash = 0;
  for (auto E : ED->enumerators()) {
    if (E->getDeclName().isIdentifier()) {
      hash ^= std::hash<std::string>()(E->getName());
    }
  }
  Ins.first->second = hash;
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

GraphObserver::NodeId
IndexerASTVisitor::BuildNodeIdForDecl(const clang::Decl *Decl, unsigned Index) {
  GraphObserver::NodeId BaseId(BuildNodeIdForDecl(Decl));
  return GraphObserver::NodeId(
      BaseId.getToken(), BaseId.getRawIdentity() + "." + std::to_string(Index));
}

MaybeFew<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForImplicitStmt(const clang::Stmt *Stmt) {
  if (!Verbosity) {
    return None();
  }
  // Do a quickish test to see if the Stmt is implicit.
  clang::ast_type_traits::DynTypedNode CurrentNode =
      clang::ast_type_traits::DynTypedNode::create(*Stmt);

  llvm::SmallVector<unsigned, 16> StmtPath;
  const clang::Decl *CurrentNodeAsDecl = nullptr;
  for (;;) {
    if ((CurrentNodeAsDecl = CurrentNode.get<clang::Decl>())) {
      if (CurrentNodeAsDecl->isImplicit()) {
        break;
      } else if (isa<clang::TranslationUnitDecl>(CurrentNodeAsDecl)) {
        CHECK(!CurrentNodeAsDecl->isImplicit());
        return None();
      }
    }
    IndexedParentVector IPV = getIndexedParents(CurrentNode);
    if (IPV.empty()) {
      break;
    }
    IndexedParent IP = IPV[0];
    StmtPath.push_back(IP.Index);
    CurrentNode = IP.Parent;
  }
  if (CurrentNodeAsDecl == nullptr) {
    // Out of luck.
    return None();
  }
  auto DeclId = BuildNodeIdForDecl(CurrentNodeAsDecl);
  std::string NewIdent = DeclId.getRawIdentity();
  {
    llvm::raw_string_ostream Ostream(NewIdent);
    for (auto &node : StmtPath) {
      Ostream << node << ".";
    }
  }
  return GraphObserver::NodeId(DeclId.getToken(), NewIdent);
}

GraphObserver::NodeId
IndexerASTVisitor::BuildNodeIdForDecl(const clang::Decl *Decl) {
  // We can't assume that no two nodes of the same Kind can appear
  // simultaneously at the same SourceLocation: witness implicit overloaded
  // operator=. We rely on names and types to disambiguate them.
  // NodeIds must be stable across analysis runs with the same input data.
  // Some NodeIds are stable in the face of changes to that data, such as
  // the IDs given to class definitions (in part because of the language rules).

  // find, not insert, since we might generate other IDs in the process of
  // generating this one (thus invalidating the iterator insert returns).
  const auto Cached = DeclToNodeId.find(Decl);
  if (Cached != DeclToNodeId.end()) {
    return Cached->second;
  }

  const auto *Token = Observer.getClaimTokenForLocation(Decl->getLocation());
  std::string Identity;
  llvm::raw_string_ostream Ostream(Identity);
  Ostream << BuildNameIdForDecl(Decl);

  // First, check to see if this thing is a builtin Decl. These things can
  // pick up weird declaration locations that aren't stable enough for us.
  if (const auto *FD = dyn_cast<FunctionDecl>(Decl)) {
    if (unsigned BuiltinID = FD->getBuiltinID()) {
      Ostream << "#builtin";
      GraphObserver::NodeId Id(Observer.getClaimTokenForBuiltin(),
                               Ostream.str());
      DeclToNodeId.insert(std::make_pair(Decl, Id));
      return Id;
    }
  }

  // Namespaces are named according to their NameIDs.
  if (const auto *NS = dyn_cast<NamespaceDecl>(Decl)) {
    Ostream << "#namespace";
    GraphObserver::NodeId Id(
        NS->isAnonymousNamespace()
            ? Observer.getAnonymousNamespaceClaimToken(NS->getLocation())
            : Observer.getNamespaceClaimToken(NS->getLocation()),
        Ostream.str());
    DeclToNodeId.insert(std::make_pair(Decl, Id));
    return Id;
  }

  // There's a special way to name type aliases.
  if (const auto *TND = dyn_cast<TypedefNameDecl>(Decl)) {
    if (auto TypedefNameId = BuildNodeIdForTypedefNameDecl(TND)) {
      DeclToNodeId.insert(std::make_pair(Decl, TypedefNameId.primary()));
      return TypedefNameId.primary();
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
      GraphObserver::NodeId Id(Token, Ostream.str());
      DeclToNodeId.insert(std::make_pair(Decl, Id));
      return Id;
    }
  } else if (const auto *Enum = dyn_cast<clang::EnumDecl>(Decl)) {
    if (Enum->getDefinition() == Enum) {
      Ostream << "#" << HashToString(SemanticHash(Enum));
      GraphObserver::NodeId Id(Token, Ostream.str());
      DeclToNodeId.insert(std::make_pair(Decl, Id));
      return Id;
    }
  } else if (const auto *ECD = dyn_cast<clang::EnumConstantDecl>(Decl)) {
    if (const auto *E = dyn_cast<clang::EnumDecl>(ECD->getDeclContext())) {
      if (E->getDefinition() == E) {
        Ostream << "#" << HashToString(SemanticHash(E));
        GraphObserver::NodeId Id(Token, Ostream.str());
        DeclToNodeId.insert(std::make_pair(Decl, Id));
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
  Ostream << "@";
  if (DeclRange.getBegin().isValid()) {
    Observer.AppendRangeToStream(
        Ostream, GraphObserver::Range(
                     DeclRange, Observer.getClaimTokenForRange(DeclRange)));
  } else {
    Ostream << "invalid";
  }
  GraphObserver::NodeId Id(Token, Ostream.str());
  DeclToNodeId.insert(std::make_pair(Decl, Id));
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
      LOG(FATAL) << "TypeLoc::" #t " unsupported";                             \
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
        if (const auto *TDType = TD->getTypeForDecl()) {
          QualType QT(TDType, 0);
          TypeSourceInfo *TSI = Context.getTrivialTypeSourceInfo(QT, L);
          return BuildNodeIdForType(TSI->getTypeLoc(), EmitRanges::No);
        } else if (const auto *TAlias = dyn_cast<TypeAliasDecl>(TD)) {
          // The names for type alias types are the same for type alias nodes.
          return BuildNodeIdForTypedefNameDecl(TAlias);
        } else {
          CHECK(IgnoreUnimplemented)
              << "Unknown case in BuildNodeIdForTemplateName";
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
        LOG(FATAL) << "Unexpected UnderlyingDecl";
      }
    } else {
      LOG(FATAL) << "BuildNodeIdForTemplateName can't identify TemplateDecl";
    }
  }
  case TemplateName::OverloadedTemplate:
    CHECK(IgnoreUnimplemented) << "TN.OverloadedTemplate";
    break;
  case TemplateName::QualifiedTemplate:
    CHECK(IgnoreUnimplemented) << "TN.QualifiedTemplate";
    break;
  case TemplateName::DependentTemplate:
    CHECK(IgnoreUnimplemented) << "TN.DependentTemplate";
    break;
  case TemplateName::SubstTemplateTemplateParm:
    CHECK(IgnoreUnimplemented) << "TN.SubstTemplateTemplateParmParm";
    break;
  case TemplateName::SubstTemplateTemplateParmPack:
    CHECK(IgnoreUnimplemented) << "TN.SubstTemplateTemplateParmPack";
    break;
  default:
    LOG(FATAL) << "Unexpected TemplateName kind!";
  }
  return None();
}

MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::BuildNodeIdForDependentName(
    const clang::NestedNameSpecifierLoc &InNNSLoc,
    const clang::DeclarationName &Id, const clang::SourceLocation IdLoc,
    const MaybeFew<GraphObserver::NodeId> &Root, EmitRanges ER) {
  if (!Verbosity) {
    return None();
  }
  // TODO(zarko): Need a better way to generate stablish names here.
  // In particular, it would be nice if a dependent name A::B::C
  // and a dependent name A::B::D were represented as ::C and ::D off
  // of the same dependent root A::B. (Does this actually make sense,
  // though? Could A::B resolve to a different entity in each case?)
  std::string Identity;
  llvm::raw_string_ostream Ostream(Identity);
  Ostream << "#nns"; // Nested name specifier.
  clang::SourceRange NNSRange(InNNSLoc.getBeginLoc(), InNNSLoc.getEndLoc());
  Ostream << "@";
  if (auto Range = ExplicitRangeInCurrentContext(NNSRange)) {
    Observer.AppendRangeToStream(Ostream, Range.primary());
  } else {
    Ostream << "invalid";
  }
  Ostream << "@";
  if (auto RCC = ExplicitRangeInCurrentContext(
          RangeForASTEntityFromSourceLocation(IdLoc))) {
    Observer.AppendRangeToStream(Ostream, RCC.primary());
  } else {
    Ostream << "invalid";
  }
  GraphObserver::NodeId IdOut(Observer.getDefaultClaimToken(), Ostream.str());
  bool HandledRecursively = false;
  unsigned SubIdCount = 0;
  clang::NestedNameSpecifierLoc NNSLoc = InNNSLoc;
  if (!NNSLoc && Root) {
    Observer.recordParamEdge(IdOut, SubIdCount++, Root.primary());
  }
  while (NNSLoc && !HandledRecursively) {
    GraphObserver::NodeId SubId(Observer.getDefaultClaimToken(), "");
    auto *NNS = NNSLoc.getNestedNameSpecifier();
    switch (NNS->getKind()) {
    case NestedNameSpecifier::Identifier: {
      // Hashcons the identifiers.
      if (auto Subtree = BuildNodeIdForDependentName(
              NNSLoc.getPrefix(), NNS->getAsIdentifier(),
              NNSLoc.getLocalBeginLoc(), Root, ER)) {
        SubId = Subtree.primary();
        HandledRecursively = true;
      } else {
        CHECK(IgnoreUnimplemented) << "NNS::Identifier";
        return None();
      }
    } break;
    case NestedNameSpecifier::Namespace:
      // TODO(zarko): Emit some representation to back this node ID.
      SubId = BuildNodeIdForDecl(NNS->getAsNamespace());
      break;
    case NestedNameSpecifier::NamespaceAlias:
      SubId = BuildNodeIdForDecl(NNS->getAsNamespaceAlias());
      break;
    case NestedNameSpecifier::TypeSpec: {
      const TypeLoc &TL = NNSLoc.getTypeLoc();
      if (auto MaybeSubId = BuildNodeIdForType(TL, ER)) {
        SubId = MaybeSubId.primary();
      } else if (auto MaybeUnlocSubId = BuildNodeIdForType(clang::QualType(
                     NNSLoc.getNestedNameSpecifier()->getAsType(), 0))) {
        SubId = MaybeUnlocSubId.primary();
      } else {
        return None();
      }
    } break;
    case NestedNameSpecifier::TypeSpecWithTemplate:
      CHECK(IgnoreUnimplemented) << "NNS::TypeSpecWithTemplate";
      return None();
    case NestedNameSpecifier::Global:
      CHECK(IgnoreUnimplemented) << "NNS::Global";
      return None();
    default:
      CHECK(IgnoreUnimplemented) << "Unexpected NestedNameSpecifier kind.";
      return None();
    }
    Observer.recordParamEdge(IdOut, SubIdCount++, SubId);
    NNSLoc = NNSLoc.getPrefix();
  }
  switch (Id.getNameKind()) {
  case clang::DeclarationName::Identifier:
    Observer.recordLookupNode(IdOut, Id.getAsIdentifierInfo()->getNameStart());
    break;
  case clang::DeclarationName::CXXConstructorName:
    Observer.recordLookupNode(IdOut, "#ctor");
    break;
  case clang::DeclarationName::CXXDestructorName:
    Observer.recordLookupNode(IdOut, "#dtor");
    break;
// TODO(zarko): Fill in the remaining relevant DeclarationName cases.
#define UNEXPECTED_DECLARATION_NAME_KIND(kind)                                 \
  case clang::DeclarationName::kind:                                           \
    CHECK(IgnoreUnimplemented) << "Unexpected DeclaraionName::" #kind;         \
    return None();
  UNEXPECTED_DECLARATION_NAME_KIND(ObjCZeroArgSelector);
  UNEXPECTED_DECLARATION_NAME_KIND(ObjCOneArgSelector);
  UNEXPECTED_DECLARATION_NAME_KIND(ObjCMultiArgSelector);
  UNEXPECTED_DECLARATION_NAME_KIND(CXXConversionFunctionName);
  UNEXPECTED_DECLARATION_NAME_KIND(CXXOperatorName);
  UNEXPECTED_DECLARATION_NAME_KIND(CXXLiteralOperatorName);
  UNEXPECTED_DECLARATION_NAME_KIND(CXXUsingDirective);
#undef UNEXPECTED_DECLARATION_NAME_KIND
  }
  if (ER == EmitRanges::Yes) {
    if (auto RCC = ExplicitRangeInCurrentContext(
            RangeForASTEntityFromSourceLocation(IdLoc))) {
      Observer.recordDeclUseLocation(RCC.primary(), IdOut);
    }
  }
  return IdOut;
}

MaybeFew<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForExpr(const clang::Expr *Expr, EmitRanges ER) {
  if (!Verbosity) {
    return None();
  }
  clang::Expr::EvalResult Result;
  std::string Identity;
  llvm::raw_string_ostream Ostream(Identity);
  std::string Text;
  llvm::raw_string_ostream TOstream(Text);
  bool IsBindingSite = false;
  auto RCC = RangeInCurrentContext(
      BuildNodeIdForImplicitStmt(Expr),
      RangeForASTEntityFromSourceLocation(Expr->getExprLoc()));
  if (!Expr->isValueDependent() && Expr->EvaluateAsRValue(Result, Context)) {
    // TODO(zarko): Represent constant values of any type as nodes in the
    // graph; link ranges to them. Right now we don't emit any node data for
    // #const signatures.
    TOstream << Result.Val.getAsString(Context, Expr->getType());
    Ostream << TOstream.str() << "#const";
  } else {
    // This includes expressions like UnresolvedLookupExpr, which can appear
    // in primary templates.
    if (!RCC) {
      return None();
    }
    Observer.AppendRangeToStream(Ostream, RCC.primary());
    Expr->printPretty(TOstream, nullptr,
                      clang::PrintingPolicy(*Observer.getLangOptions()));
    Ostream << TOstream.str();
    IsBindingSite = true;
  }
  auto ResultId =
      GraphObserver::NodeId(Observer.getDefaultClaimToken(), Ostream.str());
  if (ER == EmitRanges::Yes && RCC) {
    if (IsBindingSite) {
      Observer.recordDefinitionBindingRange(RCC.primary(), ResultId);
      Observer.recordLookupNode(ResultId, TOstream.str());
    } else {
      Observer.recordDeclUseLocation(RCC.primary(), ResultId,
                                     GraphObserver::Claimability::Claimable);
    }
  }
  return ResultId;
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
    CHECK(IgnoreUnimplemented) << "TA.Null";
    return None();
  case TemplateArgument::Type:
    CHECK(!Arg.getAsType().isNull());
    return BuildNodeIdForType(
        Context.getTrivialTypeSourceInfo(Arg.getAsType(), L)->getTypeLoc(),
        EmitRanges::No);
  case TemplateArgument::Declaration:
    CHECK(IgnoreUnimplemented) << "TA.Declaration";
    return None();
  case TemplateArgument::NullPtr:
    CHECK(IgnoreUnimplemented) << "TA.NullPtr";
    return None();
  case TemplateArgument::Integral:
    CHECK(IgnoreUnimplemented) << "TA.Integral";
    return None();
  case TemplateArgument::Template:
    return BuildNodeIdForTemplateName(Arg.getAsTemplate(), L);
  case TemplateArgument::TemplateExpansion:
    CHECK(IgnoreUnimplemented) << "TA.TemplateExpansion";
    return None();
  case TemplateArgument::Expression:
    CHECK(Arg.getAsExpr() != nullptr);
    return BuildNodeIdForExpr(Arg.getAsExpr(), EmitRanges::Yes);
  case TemplateArgument::Pack: {
    std::vector<GraphObserver::NodeId> Nodes;
    Nodes.reserve(Arg.pack_size());
    for (const auto &Element : Arg.pack_elements()) {
      auto Id(BuildNodeIdForTemplateArgument(Element, L));
      if (!Id) {
        return Id;
      }
      Nodes.push_back(Id.primary());
    }
    std::vector<const GraphObserver::NodeId *> NodePointers;
    NodePointers.reserve(Arg.pack_size());
    for (const auto &Id : Nodes) {
      NodePointers.push_back(&Id);
    }
    return Observer.recordTsigmaNode(NodePointers);
  }
  default:
    CHECK(IgnoreUnimplemented) << "Unexpected TemplateArgument kind!";
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
    CHECK(IgnoreUnimplemented) << "TA.Null";
    return None();
  case TemplateArgument::Type:
    return BuildNodeIdForType(ArgLoc.getTypeSourceInfo()->getTypeLoc(),
                              EmitRanges);
  case TemplateArgument::Declaration:
    CHECK(IgnoreUnimplemented) << "TA.Declaration";
    return None();
  case TemplateArgument::NullPtr:
    CHECK(IgnoreUnimplemented) << "TA.NullPtr";
    return None();
  case TemplateArgument::Integral:
    CHECK(IgnoreUnimplemented) << "TA.Integral";
    return None();
  case TemplateArgument::Template:
    return BuildNodeIdForTemplateName(Arg.getAsTemplate(),
                                      ArgLoc.getTemplateNameLoc());
  case TemplateArgument::TemplateExpansion:
    CHECK(IgnoreUnimplemented) << "TA.TemplateExpansion";
    return None();
  case TemplateArgument::Expression:
    CHECK(ArgLoc.getSourceExpression() != nullptr);
    return BuildNodeIdForExpr(ArgLoc.getSourceExpression(), EmitRanges);
  case TemplateArgument::Pack:
    return BuildNodeIdForTemplateArgument(Arg, ArgLoc.getLocation());
  default:
    CHECK(IgnoreUnimplemented) << "Unexpected TemplateArgument kind!";
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
  CHECK(!QT.isNull());
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
    if (EmitRanges != IndexerASTVisitor::EmitRanges::Yes ||
        !(SR.isValid() && SR.getBegin().isFileID())) {
      ID = Prev->second;
      return ID;
    }
    TypeAlreadyBuilt = true;
  }
  auto InEmitRanges = EmitRanges;
  if (!Verbosity) {
    EmitRanges = IndexerASTVisitor::EmitRanges::No;
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
  case TypeLoc::IncompleteArray: {
    const auto &T = Type.castAs<IncompleteArrayTypeLoc>();
    const auto *DT = dyn_cast<IncompleteArrayType>(PT);
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
    ID = ApplyBuiltinTypeConstructor("iarr", ElementID);
  } break;
  UNSUPPORTED_CLANG_TYPE(VariableArray);
  case TypeLoc::DependentSizedArray: {
    const auto &T = Type.castAs<DependentSizedArrayTypeLoc>();
    const auto *DT = dyn_cast<DependentSizedArrayType>(PT);
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
    if (auto ExpressionID = BuildNodeIdForExpr(
            DT ? DT->getSizeExpr() : T.getSizeExpr(), EmitRanges)) {
      ID = Observer.recordTappNode(
          Observer.getNodeIdForBuiltinType("darr"),
          {{&ElementID.primary(), &ExpressionID.primary()}});
    } else {
      ID = ApplyBuiltinTypeConstructor("darr", ElementID);
    }
  } break;
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
    EmitRanges = InEmitRanges = IndexerASTVisitor::EmitRanges::No;
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
             : Observer.recordTypeAliasNode(AliasID, AliasedTypeID.primary(),
                                            GetFormat(T.getTypedefNameDecl()));
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
      auto Parent = GetParentForFormat(SpecializedTemplateDecl);
      GraphObserver::NodeId TemplateName =
          Claimability == GraphObserver::Claimability::Unclaimable
              ? BuildNodeIdForDecl(SpecializedTemplateDecl)
              : Observer.recordNominalTypeNode(
                    BuildNameIdForDecl(SpecializedTemplateDecl),
                    GetFormat(SpecializedTemplateDecl),
                    Parent ? &Parent.primary() : nullptr);
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
          auto Parent = GetParentForFormat(Decl);
          ID = Observer.recordNominalTypeNode(
              BuildNameIdForDecl(Decl), GetFormat(Decl),
              Parent ? &Parent.primary() : nullptr);
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
        auto Parent = GetParentForFormat(Decl);
        ID = Observer.recordNominalTypeNode(
            BuildNameIdForDecl(Decl), GetFormat(Decl),
            Parent ? &Parent.primary() : nullptr);
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
    // Add anchors for parts of the NestedNameSpecifier.
    if (EmitRanges == IndexerASTVisitor::EmitRanges::Yes) {
      NestedNameSpecifierLoc NNSLoc = T.getQualifierLoc();
      while (NNSLoc) {
        NestedNameSpecifier *NNS = NNSLoc.getNestedNameSpecifier();
        if (NNS->getKind() != NestedNameSpecifier::TypeSpec &&
            NNS->getKind() != NestedNameSpecifier::TypeSpecWithTemplate)
          break;
        BuildNodeIdForType(NNSLoc.getTypeLoc(), NNS->getAsType(),
                           IndexerASTVisitor::EmitRanges::Yes);
        NNSLoc = NNSLoc.getPrefix();
      }
    }
    // Don't link 'struct'.
    EmitRanges = InEmitRanges = IndexerASTVisitor::EmitRanges::No;
    // TODO(zarko): Add an anchor for all the Elaborated type; otherwise decls
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
      CHECK_LT(TypeParm->getDepth(), TypeContext.size())
          << "Decl for type parameter missing from context.";
      CHECK_LT(TypeParm->getIndex(), TypeContext[TypeParm->getDepth()]->size())
          << "Decl for type parameter missing at specified depth.";
      const auto *ND =
          TypeContext[TypeParm->getDepth()]->getParam(TypeParm->getIndex());
      TD = cast<TemplateTypeParmDecl>(ND);
      CHECK(TypeParm->getDecl() == nullptr || TypeParm->getDecl() == TD);
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
    CHECK(!STTPT->getReplacementType().isNull());
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
    auto TemplateName = BuildNodeIdForTemplateName(
        T.getTypePtr()->getTemplateName(), T.getTemplateNameLoc());
    if (!TemplateName) {
      return TemplateName;
    }
    if (EmitRanges == IndexerASTVisitor::EmitRanges::Yes &&
        T.getTemplateNameLoc().isFileID()) {
      // Create a reference to the template instantiation that this type refers
      // to. If the type is dependent, create a reference to the primary
      // template.
      auto DeclNode = TemplateName.primary();
      auto *RD = T.getTypePtr()->getAsCXXRecordDecl();
      if (RD) {
        DeclNode = BuildNodeIdForDecl(RD);
      }
      if (auto RCC = ExplicitRangeInCurrentContext(
              RangeForSingleTokenFromSourceLocation(T.getTemplateNameLoc()))) {
        Observer.recordDeclUseLocation(RCC.primary(), DeclNode);
      }
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
        auto Parent = GetParentForFormat(Decl);
        ID = Observer.recordNominalTypeNode(
            BuildNameIdForDecl(Decl), GetFormat(Decl),
            Parent ? &Parent.primary() : nullptr);
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
      ID = BuildNodeIdForDependentName(
          NNS, clang::DeclarationName(T.getTypePtr()->getIdentifier()), NameLoc,
          None(), EmitRanges);
    }
    break;
  UNSUPPORTED_CLANG_TYPE(DependentTemplateSpecialization);
  case TypeLoc::PackExpansion: {
    const auto &T = Type.castAs<PackExpansionTypeLoc>();
    const auto *DT = dyn_cast<PackExpansionType>(PT);
    auto PatternID(BuildNodeIdForType(T.getPatternLoc(),
                                      DT ? DT->getPattern().getTypePtr()
                                         : T.getPatternLoc().getTypePtr(),
                                      EmitRanges));
    if (!PatternID) {
      return PatternID;
    }
    if (TypeAlreadyBuilt) {
      break;
    }
    ID = PatternID;
  } break;
  UNSUPPORTED_CLANG_TYPE(ObjCObject);
  UNSUPPORTED_CLANG_TYPE(ObjCInterface); // Leaf.
  UNSUPPORTED_CLANG_TYPE(ObjCObjectPointer);
  UNSUPPORTED_CLANG_TYPE(Atomic);
  default:
    // Reference, Array, Function
    LOG(FATAL) << "Incomplete pattern match on type or abstract class (?)";
  }
  if (TypeAlreadyBuilt) {
    ID = Prev->second;
  } else {
    TypeNodes[Key] = ID;
  }
  if (SR.isValid() && SR.getBegin().isFileID() &&
      InEmitRanges == IndexerASTVisitor::EmitRanges::Yes) {
    // If this is an empty SourceRange, try to expand it.
    if (SR.getBegin() == SR.getEnd()) {
      SR = RangeForASTEntityFromSourceLocation(SR.getBegin());
    }
    if (auto RCC = ExplicitRangeInCurrentContext(SR)) {
      ID.Iter([&](const GraphObserver::NodeId &I) {
        Observer.recordTypeSpellingLocation(RCC.primary(), I, Claimability);
      });
    }
  }
  return ID;
}

} // namespace kythe
