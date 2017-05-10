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
#include "GraphObserver.h"

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
#include "gflags/gflags.h"
#include "kythe/cxx/indexer/cxx/clang_utils.h"
#include "kythe/cxx/indexer/cxx/marked_source.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>

DEFINE_bool(experimental_alias_template_instantiations, false,
            "Ignore template instantation information when generating IDs.");
DEFINE_bool(experimental_threaded_claiming, false,
            "Defer answering claims and submit them in bulk when possible.");
DEFINE_bool(emit_anchors_on_builtins, true,
            "Emit anchors on builtin types like int and float.");

namespace kythe {

using namespace clang;

void *NullGraphObserver::NullClaimToken::NullClaimTokenClass = nullptr;

namespace {
/// Decides whether `Tok` can be used to quote an identifier.
bool TokenQuotesIdentifier(const clang::SourceManager &SM,
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

template <typename F>
void MapOverrideRoots(const clang::CXXMethodDecl *M, const F &Fn) {
  if (M->size_overridden_methods() == 0) {
    Fn(M);
  } else {
    for (const auto &PM : M->overridden_methods()) {
      MapOverrideRoots(PM, Fn);
    }
  }
}

template <typename F>
void MapOverrideRoots(const clang::ObjCMethodDecl *M, const F &Fn) {
  if (!M->isOverriding()) {
    Fn(M);
  } else {
    SmallVector<const ObjCMethodDecl *, 4> overrides;
    M->getOverriddenMethods(overrides);
    for (const auto &PM : overrides) {
      MapOverrideRoots(PM, Fn);
    }
  }
}

clang::QualType FollowAliasChain(const clang::TypedefNameDecl *TND) {
  clang::Qualifiers Qs;
  clang::QualType QT;
  for (;;) {
    // We'll assume that the alias chain stops as soon as we hit a non-alias.
    // This does not attempt to dereference aliases in template parameters
    // (or even aliases underneath pointers, etc).
    QT = TND->getUnderlyingType();
    Qs.addQualifiers(QT.getQualifiers());
    if (auto *TTD = dyn_cast<TypedefType>(QT.getTypePtr())) {
      TND = TTD->getDecl();
    } else {
      // We lose fidelity with getFastQualifiers.
      return QualType(QT.getTypePtr(), Qs.getFastQualifiers());
    }
  }
}

/// \brief Attempt to find the template parameters bound immediately by `DC`.
/// \return null if no parameters could be found.
clang::TemplateParameterList *GetTypeParameters(const clang::DeclContext *DC) {
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
bool IsContextSafeForLookup(const clang::DeclContext *DC) {
  if (const auto *TD = dyn_cast<clang::TagDecl>(DC)) {
    return TD->isCompleteDefinition() || TD->isBeingDefined();
  }
  return DC->isDependentContext() || !isa<clang::LinkageSpecDecl>(DC);
}

/// \brief Returns whether `Ctor` would override the in-class initializer for
/// `Field`.
bool ConstructorOverridesInitializer(const clang::CXXConstructorDecl *Ctor,
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

// Use the arithmetic sum of the pointer value of clang::Type and the numerical
// value of CVR qualifiers as the unique key for a QualType.
// The size of clang::Type is 24 (as of 2/22/2013), and the maximum of the
// qualifiers (i.e., the return value of clang::Qualifiers::getCVRQualifiers() )
// is clang::Qualifiers::CVRMask which is 7. Therefore, uniqueness is satisfied.
int64_t ComputeKeyFromQualType(const ASTContext &Context, const QualType &QT,
                               const Type *T) {
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

/// \brief Restores the type of a stacklike container of `ElementType` upon
/// destruction.
template <typename StackType>
class StackSizeRestorer {
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

/// \return true if `D` should not be visited because its name will never be
/// uttered due to aliasing rules.
bool SkipAliasedDecl(const clang::Decl *D) {
  return FLAGS_experimental_alias_template_instantiations &&
         (FindSpecializedTemplate(D) != D);
}

bool IsObjCForwardDecl(const clang::ObjCInterfaceDecl *decl) {
  return !decl->isThisDeclarationADefinition();
}
}  // anonymous namespace

bool IsClaimableForTraverse(const clang::Decl *decl) {
  // Operationally, we'll define this as any decl that causes
  // Job->UnderneathImplicitTemplateInstantiation to be set.
  if (auto *VTSD = dyn_cast<const clang::VarTemplateSpecializationDecl>(decl)) {
    return !VTSD->isExplicitInstantiationOrSpecialization();
  }
  if (auto *CTSD =
          dyn_cast<const clang::ClassTemplateSpecializationDecl>(decl)) {
    return !CTSD->isExplicitInstantiationOrSpecialization();
  }
  if (auto *FD = dyn_cast<const clang::FunctionDecl>(decl)) {
    if (const auto *MSI = FD->getMemberSpecializationInfo()) {
      // The definitions of class template member functions are not necessarily
      // dominated by the class template definition.
      if (!MSI->isExplicitSpecialization()) {
        return true;
      }
    } else if (const auto *FSI = FD->getTemplateSpecializationInfo()) {
      if (!FSI->isExplicitInstantiationOrSpecialization()) {
        return true;
      }
    }
  }
  return false;
}

enum class Prunability {
  kNone,
  kImmediate,
  kDeferIncompleteFunctions,
  kDeferred
};

/// \brief RAII class used to pair claimImplicitNode/finishImplicitNode
/// when pruning AST traversal.
class PruneCheck {
 public:
  PruneCheck(IndexerASTVisitor *visitor, const clang::Decl *decl)
      : visitor_(visitor) {
    if (!visitor_->Job->UnderneathImplicitTemplateInstantiation &&
        visitor_->declDominatesPrunableSubtree(decl)) {
      // This node in the AST dominates a subtree that can be pruned.
      if (!visitor_->Observer.claimLocation(decl->getLocation())) {
        can_prune_ = Prunability::kImmediate;
      }
      return;
    }
    if (llvm::isa<clang::FunctionDecl>(decl)) {
      // We make the assumption that we can stop traversal at function
      // declarations, which means that we always expect the subtree rooted
      // at a particular FunctionDecl under a particular type context will
      // look the same. If this isn't a function declaration, the tree we
      // see depends on external use sites. This assumption is reasonable
      // for code that doesn't rely on making different sets of decls
      // visible for ADL in different places (for example). (The core
      // difference between a function body and, say, a class body is that
      // function bodies are "closed*" and can't span files**, nor can they
      // leak local template declarations in a way that would allow
      // clients outside the function body to instantiate those declarations
      // at different types.)
      //
      //  * modulo ADL, dependent lookup, different overloads/specializations
      //    in context; code that makes these a problem is arguably not
      //    reasonable code
      // ** modulo wacky macros
      //
      GenerateCleanupId(decl);
      if (FLAGS_experimental_threaded_claiming ||
          !visitor_->Observer.claimImplicitNode(cleanup_id_)) {
        can_prune_ = Prunability::kDeferred;
      }
    } else if (llvm::isa<clang::ClassTemplateSpecializationDecl>(decl)) {
      GenerateCleanupId(decl);
      if (FLAGS_experimental_threaded_claiming ||
          !visitor_->Observer.claimImplicitNode(cleanup_id_)) {
        can_prune_ = Prunability::kDeferIncompleteFunctions;
      }
    }
  }
  ~PruneCheck() {
    if (!FLAGS_experimental_threaded_claiming && !cleanup_id_.empty()) {
      visitor_->Observer.finishImplicitNode(cleanup_id_);
    }
  }

  /// \return true if the decl supplied during construction doesn't need
  /// traversed.
  Prunability can_prune() const { return can_prune_; }

  const std::string &cleanup_id() { return cleanup_id_; }

 private:
  void GenerateCleanupId(const clang::Decl *decl) {
    // TODO(zarko): Check to see if non-function members of a class
    // can be traversed once per argument set.
    cleanup_id_ = visitor_->BuildNodeIdForDecl(decl).getRawIdentity();
    // It's critical that we distinguish between different argument lists
    // here even if aliasing is turned on; otherwise we will drop data.
    // If aliasing is off, the NodeId already contains this information.
    if (FLAGS_experimental_alias_template_instantiations) {
      clang::ast_type_traits::DynTypedNode current_node =
          clang::ast_type_traits::DynTypedNode::create(*decl);
      const clang::Decl *current_decl;
      llvm::raw_string_ostream ostream(cleanup_id_);
      while (!(current_decl = current_node.get<clang::Decl>()) ||
             !isa<clang::TranslationUnitDecl>(current_decl)) {
        IndexedParent *parent = visitor_->getIndexedParent(current_node);
        if (parent == nullptr) {
          break;
        }
        current_node = parent->Parent;
        if (!current_decl) {
          continue;
        }
        if (const auto *czdecl =
                dyn_cast<ClassTemplateSpecializationDecl>(current_decl)) {
          ostream << "#"
                  << HashToString(visitor_->SemanticHash(
                         &czdecl->getTemplateInstantiationArgs()));
        } else if (const auto *fdecl = dyn_cast<FunctionDecl>(current_decl)) {
          if (const auto *template_args =
                  fdecl->getTemplateSpecializationArgs()) {
            ostream << "#"
                    << HashToString(visitor_->SemanticHash(template_args));
          }
        } else if (const auto *vdecl =
                       dyn_cast<VarTemplateSpecializationDecl>(current_decl)) {
          ostream << "#"
                  << HashToString(visitor_->SemanticHash(
                         &vdecl->getTemplateInstantiationArgs()));
        }
      }
    }
    cleanup_id_ = CompressString(cleanup_id_);
  }
  Prunability can_prune_ = Prunability::kNone;
  std::string cleanup_id_;
  IndexerASTVisitor *visitor_;
};

void IndexerASTVisitor::deleteAllParents() {
  if (!AllParents) {
    return;
  }
  for (const auto &Entry : *AllParents) {
    delete Entry.second.getPointer();
  }
  AllParents.reset(nullptr);
}

IndexedParent *IndexerASTVisitor::getIndexedParent(
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
    return nullptr;
  }
  return I->second.getPointer();
}

bool IndexerASTVisitor::declDominatesPrunableSubtree(const clang::Decl *Decl) {
  const auto Node = clang::ast_type_traits::DynTypedNode::create(*Decl);
  if (!AllParents) {
    ProfileBlock block(Observer.getProfilingCallback(), "build_parent_map");
    AllParents =
        IndexedParentASTVisitor::buildMap(*Context.getTranslationUnitDecl());
  }
  IndexedParentMap::const_iterator I =
      AllParents->find(Node.getMemoizationData());
  if (I == AllParents->end()) {
    // Safe default.
    return false;
  }
  return I->second.getInt() == 0;
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

SourceRange IndexerASTVisitor::ConsumeToken(
    SourceLocation StartLocation, clang::tok::TokenKind ExpectedKind) const {
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
      const SourceLocation Begin = Token.getLocation();
      return SourceRange(
          Begin, GetLocForEndOfToken(*Observer.getSourceManager(),
                                     *Observer.getLangOptions(), Begin));
    }
  }
  return SourceRange();  // invalid location signals error/mismatch.
}

clang::SourceRange IndexerASTVisitor::RangeForNameOfDeclaration(
    const clang::NamedDecl *Decl) const {
  const SourceLocation StartLocation = Decl->getLocation();
  if (StartLocation.isInvalid()) {
    return SourceRange();
  }
  if (StartLocation.isFileID()) {
    if (isa<clang::CXXDestructorDecl>(Decl)) {
      // If the first token is "~" (or its alternate spelling, "compl") and
      // the second is the name of class (rather than the name of a macro),
      // then span two tokens.  Otherwise span just one.
      const SourceLocation NextLocation =
          ConsumeToken(StartLocation, clang::tok::tilde).getEnd();

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
          const SourceLocation EndLocation = GetLocForEndOfToken(
              *Observer.getSourceManager(), *Observer.getLangOptions(),
              SecondToken.getLocation());
          return clang::SourceRange(StartLocation, EndLocation);
        }
      }
    } else if (auto *M = dyn_cast<clang::ObjCMethodDecl>(Decl)) {
      // Only take the first selector (if we have one). This simplifies what
      // consumers of this data have to do but it is not correct.

      if (M->getNumSelectorLocs() == 0) {
        // Take the whole declaration. For decls this goes up to but does not
        // include the ";". For definitions this goes up to but does not include
        // the "{". This range will include other fields, such as the return
        // type, parameter types, and parameter names.

        auto S = M->getSelectorStartLoc();
        auto E = M->getDeclaratorEndLoc();
        return SourceRange(S, E);
      }
      // TODO(salguarnieri) Return multiple ranges, one for each portion of the
      // selector.
      const SourceLocation &Loc = M->getSelectorLoc(0);
      if (Loc.isValid() && Loc.isFileID()) {
        return RangeForSingleTokenFromSourceLocation(
            *Observer.getSourceManager(), *Observer.getLangOptions(), Loc);
      }

      // If the selector location is not valid or is not a file, return the
      // whole range of the selector and hope for the best.
      LogErrorWithASTDump("Could not get source range", M);
      return M->getSourceRange();
    }
  }
  return RangeForASTEntityFromSourceLocation(
      *Observer.getSourceManager(), *Observer.getLangOptions(), StartLocation);
}

void IndexerASTVisitor::MaybeRecordDefinitionRange(
    const MaybeFew<GraphObserver::Range> &R, const GraphObserver::NodeId &Id) {
  if (R) {
    Observer.recordDefinitionBindingRange(R.primary(), Id);
  }
}

void IndexerASTVisitor::RecordCallEdges(const GraphObserver::Range &Range,
                                        const GraphObserver::NodeId &Callee) {
  if (!Job->BlameStack.empty()) {
    for (const auto &Caller : Job->BlameStack.back()) {
      Observer.recordCallEdge(Range, Caller, Callee);
    }
  }
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
    Done,     ///< Lookup is terminated; don't add any more tokens.
    Updated,  ///< The last token narrowed down the lookup.
    Progress  ///< The last token didn't narrow anything down.
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
          for (const auto &Param : FunctionContext->parameters()) {
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
  WaitingForMark,  ///< We're waiting for something that might denote an
                   ///< embedded identifier.
  SawCommand       ///< We saw something Clang identifies as a comment command.
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

void IndexerASTVisitor::HandleFileLevelComments(
    clang::FileID Id, const GraphObserver::NodeId &FileNode) {
  const auto &RCL = Context.getRawCommentList();
  auto IdStart = Context.getSourceManager().getLocForStartOfFile(Id);
  if (!IdStart.isFileID() || !IdStart.isValid()) {
    return;
  }
  auto StartIdLoc = Context.getSourceManager().getDecomposedLoc(IdStart);
  // Find the block of comments for the given file. This behavior is not well-
  // defined by Clang, which commits only to the RawComments being
  // "sorted in order of appearance in the translation unit".
  if (RCL.getComments().empty()) {
    return;
  }
  // Find the first RawComment whose start location is greater or equal to
  // the start of the file whose FileID is Id.
  auto C = std::lower_bound(
      RCL.getComments().begin(), RCL.getComments().end(), StartIdLoc,
      [&](clang::RawComment *const T1, const decltype(StartIdLoc) &T2) {
        return Context.getSourceManager().getDecomposedLoc(T1->getLocStart()) <
               T2;
      });
  // Walk through the comments in Id starting with the one at the top. If we
  // ever leave Id, then we're done. (The first time around the loop, if C isn't
  // already in Id, this check will immediately break;.)
  if (C != RCL.getComments().end()) {
    auto CommentIdLoc =
        Context.getSourceManager().getDecomposedLoc((*C)->getLocStart());
    if (CommentIdLoc.first != Id) {
      return;
    }
    // Here's a simple heuristic: the first comment in a file is the file-level
    // comment. This is bad for files with (e.g.) license blocks, but we can
    // gradually refine as necessary.
    if (VisitedComments.find(*C) == VisitedComments.end()) {
      VisitComment(*C, Context.getTranslationUnitDecl(), FileNode);
    }
    return;
  }
}

void IndexerASTVisitor::VisitComment(
    const clang::RawComment *Comment, const clang::DeclContext *DC,
    const GraphObserver::NodeId &DocumentedNode) {
  VisitedComments.insert(Comment);
  const auto &SM = *Observer.getSourceManager();
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
        auto ResultId = BuildNodeIdForRefToDecl(Result);
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
    Observer.recordDocumentationRange(RCC.primary(), DocumentedNode);
    Observer.recordDocumentationText(DocumentedNode, StrippedRawText,
                                     LinkNodes);
  }
}

bool IndexerASTVisitor::VisitDecl(const clang::Decl *Decl) {
  if (Job->UnderneathImplicitTemplateInstantiation || Decl == nullptr) {
    // Template instantiation can't add any documentation text.
    return true;
  }
  const auto *Comment = Context.getRawCommentForDeclNoCache(Decl);
  if (!Comment) {
    // Fast path: if there are no attached documentation comments, bail.
    return true;
  }
  const auto *DCxt = dyn_cast<DeclContext>(Decl);
  if (!DCxt) {
    DCxt = Decl->getDeclContext();
    if (!DCxt) {
      DCxt = Context.getTranslationUnitDecl();
    }
  }

  if (const auto *DC = dyn_cast_or_null<DeclContext>(Decl)) {
    if (auto DCID = BuildNodeIdForDeclContext(DC)) {
      VisitComment(Comment, DCxt, DCID.primary());
    }
    if (const auto *CTPSD =
            dyn_cast_or_null<ClassTemplatePartialSpecializationDecl>(Decl)) {
      auto NodeId = BuildNodeIdForDecl(CTPSD);
      VisitComment(Comment, DCxt, NodeId);
    }
    if (const auto *FD = dyn_cast_or_null<FunctionDecl>(Decl)) {
      if (const auto *FTD = FD->getDescribedFunctionTemplate()) {
        auto NodeId = BuildNodeIdForDecl(FTD);
        VisitComment(Comment, DCxt, NodeId);
      }
    }
  } else {
    if (const auto *VD = dyn_cast_or_null<VarDecl>(Decl)) {
      if (const auto *VTD = VD->getDescribedVarTemplate()) {
        auto NodeId = BuildNodeIdForDecl(VTD);
        VisitComment(Comment, DCxt, NodeId);
      }
    } else if (const auto *AD = dyn_cast_or_null<TypeAliasDecl>(Decl)) {
      if (const auto *TATD = AD->getDescribedAliasTemplate()) {
        auto NodeId = BuildNodeIdForDecl(TATD);
        VisitComment(Comment, DCxt, NodeId);
      }
    }
    auto NodeId = BuildNodeIdForDecl(Decl);
    VisitComment(Comment, DCxt, NodeId);
  }
  return true;
}

bool IndexerASTVisitor::TraverseDecl(clang::Decl *Decl) {
  if (ShouldStopIndexing()) {
    return false;
  }
  if (Decl == nullptr) {
    return true;
  }
  struct RestoreBool {
    RestoreBool(bool *to_restore)
        : to_restore_(to_restore), state_(*to_restore) {}
    ~RestoreBool() { *to_restore_ = state_; }
    bool *to_restore_;
    bool state_;
  } RB(&Job->PruneIncompleteFunctions);
  if (Job->PruneIncompleteFunctions) {
    if (auto *FD = llvm::dyn_cast<clang::FunctionDecl>(Decl)) {
      if (!FD->isThisDeclarationADefinition()) {
        return true;
      }
    } else {
      return true;
    }
  }
  if (FLAGS_experimental_threaded_claiming) {
    if (Decl != Job->Decl) {
      PruneCheck Prune(this, Decl);
      auto can_prune = Prune.can_prune();
      if (can_prune == Prunability::kImmediate) {
        return true;
      } else if (can_prune != Prunability::kNone) {
        Worklist->EnqueueJobForImplicitDecl(
            Decl, can_prune == Prunability::kDeferIncompleteFunctions,
            Prune.cleanup_id());
        return true;
      }
    } else {
      if (Job->SetPruneIncompleteFunctions) {
        Job->PruneIncompleteFunctions = true;
      }
    }
  } else {
    PruneCheck Prune(this, Decl);
    auto can_prune = Prune.can_prune();
    if (can_prune == Prunability::kImmediate) {
      return true;
    } else if (can_prune == Prunability::kDeferIncompleteFunctions) {
      Job->PruneIncompleteFunctions = true;
    }
  }
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
    auto R = RestoreStack(Job->BlameStack);
    auto S = RestoreStack(Job->RangeContext);

    if (FD->isTemplateInstantiation() &&
        FD->getTemplateSpecializationKind() !=
            clang::TSK_ExplicitSpecialization) {
      // Explicit specializations have ranges.
      if (const auto RangeId = BuildNodeIdForRefToDeclContext(FD)) {
        Job->RangeContext.push_back(RangeId.primary());
      } else {
        Job->RangeContext.push_back(BuildNodeIdForDecl(FD));
      }
    }
    if (const auto BlameId = BuildNodeIdForDeclContext(FD)) {
      Job->BlameStack.push_back(IndexJob::SomeNodes(1, BlameId.primary()));
    } else {
      Job->BlameStack.push_back(
          IndexJob::SomeNodes(1, BuildNodeIdForRefToDecl(FD)));
    }

    // Dispatch the remaining logic to the base class TraverseDecl() which will
    // call TraverseX(X*) for the most-derived X.
    return RecursiveASTVisitor::TraverseDecl(FD);
  } else if (auto *ID = dyn_cast_or_null<clang::FieldDecl>(Decl)) {
    // This will also cover the case of clang::ObjCIVarDecl since it is a
    // subclass.
    if (ID->hasInClassInitializer()) {
      // Blame calls from in-class initializers I on all ctors C of the
      // containing class so long as C does not have its own initializer for
      // I's field.
      auto R = RestoreStack(Job->BlameStack);
      if (auto *CR = dyn_cast_or_null<clang::CXXRecordDecl>(ID->getParent())) {
        if ((CR = CR->getDefinition())) {
          IndexJob::SomeNodes Ctors;
          auto TryCtor = [&](const clang::CXXConstructorDecl *Ctor) {
            if (!ConstructorOverridesInitializer(Ctor, ID)) {
              if (const auto BlameId = BuildNodeIdForRefToDeclContext(Ctor)) {
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
            Job->BlameStack.push_back(Ctors);
          }
        }
      }
      return RecursiveASTVisitor::TraverseDecl(ID);
    }
  } else if (auto *MD = dyn_cast_or_null<clang::ObjCMethodDecl>(Decl)) {
    // These variables (R and S) clean up the stacks (Job->BlameStack and
    // Job->RangeContext) when the local variables (R and S) are
    // destructed.
    auto R = RestoreStack(Job->BlameStack);
    auto S = RestoreStack(Job->RangeContext);

    if (const auto BlameId = BuildNodeIdForDeclContext(MD)) {
      Job->BlameStack.push_back(IndexJob::SomeNodes(1, BlameId.primary()));
    } else {
      Job->BlameStack.push_back(IndexJob::SomeNodes(1, BuildNodeIdForDecl(MD)));
    }

    // Dispatch the remaining logic to the base class TraverseDecl() which will
    // call TraverseX(X*) for the most-derived X.
    return RecursiveASTVisitor::TraverseDecl(MD);
  }

  return RecursiveASTVisitor::TraverseDecl(Decl);
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
    if (BuildTemplateArgumentList(
            llvm::makeArrayRef(E->getTemplateArgs(), E->getNumTemplateArgs()),
            llvm::None, ArgIds, ArgNodeIds)) {
      auto TappNodeId = Observer.recordTappNode(DepNodeId.primary(), ArgNodeIds,
                                                ArgNodeIds.size());
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
    auto Range = RangeForASTEntityFromSourceLocation(
        *Observer.getSourceManager(), *Observer.getLangOptions(),
        E->getMemberLoc());
    auto StmtId = BuildNodeIdForImplicitStmt(E);
    if (auto RCC = RangeInCurrentContext(StmtId, Range)) {
      Observer.recordDeclUseLocation(RCC.primary(),
                                     BuildNodeIdForRefToDecl(FieldDecl),
                                     GraphObserver::Claimability::Unclaimable);
      if (E->hasExplicitTemplateArgs()) {
        // We still want to link the template args.
        std::vector<GraphObserver::NodeId> ArgIds;
        std::vector<const GraphObserver::NodeId *> ArgNodeIds;
        BuildTemplateArgumentList(
            llvm::makeArrayRef(E->getTemplateArgs(), E->getNumTemplateArgs()),
            llvm::None, ArgIds, ArgNodeIds);
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
    if (!Job->BlameStack.empty()) {
      clang::SourceLocation RPL = E->getParenOrBraceRange().getEnd();
      clang::SourceRange SR = E->getSourceRange();
      if (RPL.isValid()) {
        // This loses the right paren without the offset.
        SR.setEnd(RPL.getLocWithOffset(1));
      }
      auto StmtId = BuildNodeIdForImplicitStmt(E);
      if (auto RCC = RangeInCurrentContext(StmtId, SR)) {
        RecordCallEdges(RCC.primary(), BuildNodeIdForRefToDecl(Callee));
      }
    }
  }
  return true;
}

bool IndexerASTVisitor::VisitCXXDeleteExpr(const clang::CXXDeleteExpr *E) {
  if (Job->BlameStack.empty()) {
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
      DDId = BuildNodeIdForRefToDecl(DD);
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

bool IndexerASTVisitor::VisitCXXNewExpr(const clang::CXXNewExpr *E) {
  auto StmtId = BuildNodeIdForImplicitStmt(E);
  if (FunctionDecl *New = E->getOperatorNew()) {
    auto NewId = BuildNodeIdForRefToDecl(New);
    clang::SourceLocation NewLoc = E->getLocStart();
    if (NewLoc.isFileID()) {
      clang::SourceRange NewRange(
          NewLoc, GetLocForEndOfToken(*Observer.getSourceManager(),
                                      *Observer.getLangOptions(), NewLoc));
      if (auto RCC = RangeInCurrentContext(StmtId, NewRange)) {
        Observer.recordDeclUseLocation(
            RCC.primary(), NewId, GraphObserver::Claimability::Unclaimable);
      }
    }
  }

  BuildNodeIdForType(E->getAllocatedTypeSourceInfo()->getTypeLoc(),
                     E->getAllocatedTypeSourceInfo()->getTypeLoc().getType(),
                     EmitRanges::Yes);
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
    SR.setEnd(RangeForASTEntityFromSourceLocation(*Observer.getSourceManager(),
                                                  *Observer.getLangOptions(),
                                                  SR.getEnd())
                  .getEnd());
    auto StmtId = BuildNodeIdForImplicitStmt(E);
    if (auto RCC = RangeInCurrentContext(StmtId, SR)) {
      RecordCallEdges(RCC.primary(), DDId.primary());
    }
  }
  return true;
}

bool IndexerASTVisitor::VisitCXXUnresolvedConstructExpr(
    const clang::CXXUnresolvedConstructExpr *E) {
  if (!Job->BlameStack.empty()) {
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
  if (Job->BlameStack.empty()) {
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
      auto CalleeId = BuildNodeIdForRefToDecl(Callee);
      RecordCallEdges(RCC.primary(), CalleeId);
      for (const auto &S : Supports) {
        S->InspectCallExpr(*this, E, RCC.primary(), CalleeId);
      }
    } else if (const auto *CE = E->getCallee()) {
      if (auto CalleeId = BuildNodeIdForExpr(CE, EmitRanges::Yes)) {
        RecordCallEdges(RCC.primary(), CalleeId.primary());
      }
    }
  }
  return true;
}

MaybeFew<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForImplicitTemplateInstantiation(
    const clang::Decl *Decl) {
  std::vector<GraphObserver::NodeId> NIDS;
  std::vector<const GraphObserver::NodeId *> NIDPS;
  const clang::TemplateArgumentLoc *ArgsAsWritten = nullptr;
  unsigned NumArgsAsWritten = 0;
  const clang::TemplateArgumentList *Args = nullptr;

  if (const auto *FD = dyn_cast<clang::FunctionDecl>(Decl)) {
    if (FD->getDescribedFunctionTemplate() != nullptr) {
      // This is the body of a function template.
      return None();
    }
    if (auto *MSI = FD->getMemberSpecializationInfo()) {
      if (MSI->isExplicitSpecialization()) {
        // Refer to explicit specializations directly.
        return None();
      }
      // This is a member under a template instantiation. See T230.
      return Observer.recordTappNode(
          BuildNodeIdForDecl(MSI->getInstantiatedFrom()), {}, 0);
    }
    std::vector<std::pair<clang::TemplateName, clang::SourceLocation>> TNs;
    if (auto *FTSI = FD->getTemplateSpecializationInfo()) {
      if (FTSI->isExplicitSpecialization()) {
        // Refer to explicit specializations directly.
        return None();
      }
      if (FTSI->TemplateArgumentsAsWritten) {
        // We have source locations for the template arguments.
        ArgsAsWritten = FTSI->TemplateArgumentsAsWritten->getTemplateArgs();
        NumArgsAsWritten = FTSI->TemplateArgumentsAsWritten->NumTemplateArgs;
      }
      Args = FTSI->TemplateArguments;
      TNs.emplace_back(TemplateName(FTSI->getTemplate()),
                       FTSI->getPointOfInstantiation());
    } else if (auto *DFTSI = FD->getDependentSpecializationInfo()) {
      ArgsAsWritten = DFTSI->getTemplateArgs();
      NumArgsAsWritten = DFTSI->getNumTemplateArgs();
      for (unsigned T = 0; T < DFTSI->getNumTemplates(); ++T) {
        TNs.emplace_back(clang::TemplateName(DFTSI->getTemplate(T)),
                         FD->getLocation());
      }
    }
    // We can't do anything useful if we don't have type arguments.
    if (ArgsAsWritten || Args) {
      bool CouldGetAllTypes = true;
      if (ArgsAsWritten) {
        // Prefer arguments as they were written in source files.
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
        // If there's more than one possible template name (e.g., this is
        // dependent), choose one arbitrarily.
        for (const auto &TN : TNs) {
          if (auto SpecializedNode =
                  BuildNodeIdForTemplateName(TN.first, TN.second)) {
            return Observer.recordTappNode(SpecializedNode.primary(), NIDPS,
                                           NumArgsAsWritten);
          }
        }
      }
    }
  }
  // TODO(zarko): Variables and records.
  return None();
}

GraphObserver::NodeId IndexerASTVisitor::BuildNodeIdForRefToDecl(
    const clang::Decl *Decl) {
  if (auto TappId = BuildNodeIdForImplicitTemplateInstantiation(Decl)) {
    return TappId.primary();
  }
  return BuildNodeIdForDecl(Decl);
}

MaybeFew<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForRefToDeclContext(
    const clang::DeclContext *DC) {
  if (auto *DCDecl = llvm::dyn_cast<const clang::Decl>(DC)) {
    if (auto TappId = BuildNodeIdForImplicitTemplateInstantiation(DCDecl)) {
      return TappId;
    }
    return BuildNodeIdForDeclContext(DC);
  }
  return None();
}

MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::BuildNodeIdForDeclContext(
    const clang::DeclContext *DC) {
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
    if (FLAGS_experimental_alias_template_instantiations) {
      if (auto ContextId = BuildNodeIdForRefToDeclContext(DC)) {
        Observer.recordChildOfEdge(DeclNode, ContextId.primary());
      }
    } else {
      if (auto ContextId = BuildNodeIdForDeclContext(DC)) {
        Observer.recordChildOfEdge(DeclNode, ContextId.primary());
      }
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
  if (!VisitDeclRefOrIvarRefExpr(DRE, DRE->getDecl(), DRE->getLocation())) {
    return false;
  }
  if (DRE->hasQualifier()) {
    VisitNestedNameSpecifierLoc(DRE->getQualifierLoc());
  }
  for (auto &ArgLoc : DRE->template_arguments()) {
    BuildNodeIdForTemplateArgument(ArgLoc, EmitRanges::Yes);
  }
  return true;
}

void IndexerASTVisitor::VisitNestedNameSpecifierLoc(
    clang::NestedNameSpecifierLoc NNSL) {
  while (NNSL) {
    NestedNameSpecifier *NNS = NNSL.getNestedNameSpecifier();
    if (NNS->getKind() != NestedNameSpecifier::TypeSpec &&
        NNS->getKind() != NestedNameSpecifier::TypeSpecWithTemplate)
      break;
    BuildNodeIdForType(NNSL.getTypeLoc(), NNS->getAsType(),
                       IndexerASTVisitor::EmitRanges::Yes);
    NNSL = NNSL.getPrefix();
  }
}

// Use FoundDecl to get to template defs; use getDecl to get to template
// instantiations.
bool IndexerASTVisitor::VisitDeclRefOrIvarRefExpr(
    const clang::Expr *Expr, const NamedDecl *const FoundDecl,
    SourceLocation SL) {
  // TODO(zarko): check to see if this DeclRefExpr has already been indexed.
  // (Use a simple N=1 cache.)
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
  if (SL.isValid()) {
    SourceRange Range = RangeForASTEntityFromSourceLocation(
        *Observer.getSourceManager(), *Observer.getLangOptions(), SL);
    auto StmtId = BuildNodeIdForImplicitStmt(Expr);
    if (auto RCC = RangeInCurrentContext(StmtId, Range)) {
      GraphObserver::NodeId DeclId = BuildNodeIdForRefToDecl(TargetDecl);
      Observer.recordDeclUseLocation(RCC.primary(), DeclId,
                                     GraphObserver::Claimability::Unclaimable);
      for (const auto &S : Supports) {
        S->InspectDeclRef(*this, SL, RCC.primary(), DeclId, TargetDecl);
      }
    }
  }
  return true;
}

bool IndexerASTVisitor::BuildTemplateArgumentList(
    Optional<ArrayRef<TemplateArgumentLoc>> ArgsAsWritten,
    ArrayRef<TemplateArgument> Args, std::vector<GraphObserver::NodeId> &ArgIds,
    std::vector<const GraphObserver::NodeId *> &ArgNodeIds) {
  ArgIds.clear();
  ArgNodeIds.clear();
  if (ArgsAsWritten) {
    ArgIds.reserve(ArgsAsWritten->size());
    ArgNodeIds.reserve(ArgsAsWritten->size());
    for (const auto &ArgLoc : *ArgsAsWritten) {
      if (auto ArgId =
              BuildNodeIdForTemplateArgument(ArgLoc, EmitRanges::Yes)) {
        ArgIds.push_back(ArgId.primary());
      } else {
        return false;
      }
    }
  } else {
    ArgIds.reserve(Args.size());
    ArgNodeIds.reserve(Args.size());
    for (const auto &Arg : Args) {
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
  if (SkipAliasedDecl(Decl)) {
    return true;
  }
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
  auto Marks = MarkedSources.Generate(Decl);
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
    Marks.set_implicit(Job->UnderneathImplicitTemplateInstantiation ||
                       !VTSD->isExplicitInstantiationOrSpecialization());
    // If this is a partial specialization, we've already recorded the newly
    // abstracted parameters above. We can now record the type arguments passed
    // to the template we're specializing. Synthesize the type we need.
    std::vector<GraphObserver::NodeId> NIDS;
    std::vector<const GraphObserver::NodeId *> NIDPS;
    auto PrimaryOrPartial = VTSD->getSpecializedTemplateOrPartial();
    if (BuildTemplateArgumentList(
            ArgsAsWritten ? llvm::makeArrayRef(ArgsAsWritten->getTemplateArgs(),
                                               ArgsAsWritten->NumTemplateArgs)
                          : Optional<ArrayRef<TemplateArgumentLoc>>(),
            VTSD->getTemplateArgs().asArray(), NIDS, NIDPS)) {
      if (auto SpecializedNode = BuildNodeIdForTemplateName(
              clang::TemplateName(VTSD->getSpecializedTemplate()),
              VTSD->getPointOfInstantiation())) {
        if (PrimaryOrPartial.is<clang::VarTemplateDecl *>() &&
            !isa<const clang::VarTemplatePartialSpecializationDecl>(Decl)) {
          // This is both an instance and a specialization of the primary
          // template. We use the same arguments list for both.
          Observer.recordInstEdge(
              DeclNode,
              Observer.recordTappNode(SpecializedNode.primary(), NIDPS,
                                      ArgsAsWritten
                                          ? ArgsAsWritten->NumTemplateArgs
                                          : NIDPS.size()),
              GraphObserver::Confidence::NonSpeculative);
        }
        Observer.recordSpecEdge(
            DeclNode,
            Observer.recordTappNode(
                SpecializedNode.primary(), NIDPS,
                ArgsAsWritten ? ArgsAsWritten->NumTemplateArgs : NIDPS.size()),
            GraphObserver::Confidence::NonSpeculative);
      }
    }
    if (const auto *Partial =
            PrimaryOrPartial
                .dyn_cast<clang::VarTemplatePartialSpecializationDecl *>()) {
      if (BuildTemplateArgumentList(
              llvm::None, VTSD->getTemplateInstantiationArgs().asArray(), NIDS,
              NIDPS)) {
        Observer.recordInstEdge(
            DeclNode,
            Observer.recordTappNode(BuildNodeIdForDecl(Partial), NIDPS,
                                    NIDPS.size()),
            GraphObserver::Confidence::NonSpeculative);
      }
    }
  }

  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode);
  Marks.set_marked_source_end(GetLocForEndOfToken(
      *Observer.getSourceManager(), *Observer.getLangOptions(),
      Decl->getSourceRange().getEnd()));
  Marks.set_name_range(NameRange);
  if (const auto *TSI = Decl->getTypeSourceInfo()) {
    // TODO(zarko): Storage classes.
    AscribeSpelledType(TSI->getTypeLoc(), Decl->getType(), BodyDeclNode);
  } else if (auto TyNodeId = BuildNodeIdForType(Decl->getType())) {
    Observer.recordTypeEdge(BodyDeclNode, TyNodeId.primary());
  }
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  std::vector<LibrarySupport::Completion> Completions;
  if (!IsDefinition(Decl)) {
    Observer.recordVariableNode(BodyDeclNode,
                                GraphObserver::Completeness::Incomplete,
                                GraphObserver::VariableSubkind::None, None());
    Observer.recordMarkedSource(DeclNode, Marks.GenerateMarkedSource(DeclNode));
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
  Observer.recordVariableNode(BodyDeclNode,
                              GraphObserver::Completeness::Definition,
                              GraphObserver::VariableSubkind::None, None());
  Observer.recordMarkedSource(DeclNode, Marks.GenerateMarkedSource(DeclNode));
  for (const auto &S : Supports) {
    S->InspectVariable(*this, DeclNode, BodyDeclNode, Decl,
                       GraphObserver::Completeness::Definition, Completions);
  }
  return true;
}

bool IndexerASTVisitor::VisitNamespaceDecl(const clang::NamespaceDecl *Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  // Use the range covering `namespace` for anonymous namespaces.
  SourceRange NameRange;
  if (Decl->isAnonymousNamespace()) {
    SourceLocation Loc = Decl->getLocStart();
    if (Decl->isInline() && Loc.isValid() && Loc.isFileID()) {
      // Skip the `inline` keyword.
      Loc = RangeForSingleTokenFromSourceLocation(
                *Observer.getSourceManager(), *Observer.getLangOptions(), Loc)
                .getEnd();
      if (Loc.isValid() && Loc.isFileID()) {
        SkipWhitespace(*Observer.getSourceManager(), &Loc);
      }
    }
    if (Loc.isValid() && Loc.isFileID()) {
      NameRange = RangeForASTEntityFromSourceLocation(
          *Observer.getSourceManager(), *Observer.getLangOptions(), Loc);
    }
  } else {
    NameRange = RangeForNameOfDeclaration(Decl);
  }
  // Namespaces are never defined; they are only invoked.
  if (auto RCC =
          RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange)) {
    Observer.recordDeclUseLocation(RCC.primary(), DeclNode,
                                   GraphObserver::Claimability::Unclaimable);
  }
  Observer.recordNamespaceNode(DeclNode, Marks.GenerateMarkedSource(DeclNode));
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  return true;
}

bool IndexerASTVisitor::VisitFieldDecl(const clang::FieldDecl *Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode);
  Marks.set_implicit(Job->UnderneathImplicitTemplateInstantiation);
  Marks.set_marked_source_end(GetLocForEndOfToken(
      *Observer.getSourceManager(), *Observer.getLangOptions(),
      Decl->getSourceRange().getEnd()));
  Marks.set_name_range(NameRange);
  // TODO(zarko): Record completeness data. This is relevant for static fields,
  // which may be declared along with a complete class definition but later
  // defined in a separate translation unit.
  Observer.recordVariableNode(DeclNode, GraphObserver::Completeness::Definition,
                              GraphObserver::VariableSubkind::Field,
                              Marks.GenerateMarkedSource(DeclNode));
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
  auto Marks = MarkedSources.Generate(Decl);
  Marks.set_implicit(Job->UnderneathImplicitTemplateInstantiation);
  // We first build the NameId and NodeId for the enumerator.
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  SourceLocation DeclLoc = Decl->getLocation();
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  Marks.set_marked_source_end(GetLocForEndOfToken(
      *Observer.getSourceManager(), *Observer.getLangOptions(),
      Decl->getSourceRange().getEnd()));
  Marks.set_name_range(NameRange);
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode);
  Observer.recordIntegerConstantNode(DeclNode, Decl->getInitVal());
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  Observer.recordMarkedSource(DeclNode, Marks.GenerateMarkedSource(DeclNode));
  return true;
}

bool IndexerASTVisitor::VisitEnumDecl(const clang::EnumDecl *Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  Marks.set_implicit(Job->UnderneathImplicitTemplateInstantiation);
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  SourceLocation DeclLoc = Decl->getLocation();
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  if (Decl->isThisDeclarationADefinition() && Decl->hasBody()) {
    Marks.set_marked_source_end(Decl->getBody()->getSourceRange().getBegin());
  } else {
    Marks.set_marked_source_end(GetLocForEndOfToken(
        *Observer.getSourceManager(), *Observer.getLangOptions(),
        Decl->getSourceRange().getEnd()));
  }
  Marks.set_name_range(NameRange);
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode);
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
  Observer.recordMarkedSource(DeclNode, Marks.GenerateMarkedSource(DeclNode));
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
  Job->TypeContext.push_back(TD->getTemplateParameters());
  bool Result =
      RecursiveASTVisitor<IndexerASTVisitor>::TraverseClassTemplateDecl(TD);
  Job->TypeContext.pop_back();
  return Result;
}

// NB: The Traverse* member that's called is based on the dynamic type of the
// AST node it's being called with (so only one of
// TraverseClassTemplate{Partial}SpecializationDecl will be called).
bool IndexerASTVisitor::TraverseClassTemplateSpecializationDecl(
    clang::ClassTemplateSpecializationDecl *TD) {
  auto R = RestoreStack(Job->RangeContext);
  bool UITI = Job->UnderneathImplicitTemplateInstantiation;
  // If this specialization was spelled out in the file, it has
  // physical ranges.
  if (TD->getTemplateSpecializationKind() !=
      clang::TSK_ExplicitSpecialization) {
    Job->RangeContext.push_back(BuildNodeIdForDecl(TD));
  }
  if (!TD->isExplicitInstantiationOrSpecialization()) {
    Job->UnderneathImplicitTemplateInstantiation = true;
  }
  bool Result = RecursiveASTVisitor<
      IndexerASTVisitor>::TraverseClassTemplateSpecializationDecl(TD);
  Job->UnderneathImplicitTemplateInstantiation = UITI;
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
  auto R = RestoreStack(Job->RangeContext);
  bool UITI = Job->UnderneathImplicitTemplateInstantiation;
  // If this specialization was spelled out in the file, it has
  // physical ranges.
  if (TD->getTemplateSpecializationKind() !=
      clang::TSK_ExplicitSpecialization) {
    Job->RangeContext.push_back(BuildNodeIdForDecl(TD));
  }
  if (!TD->isExplicitInstantiationOrSpecialization()) {
    Job->UnderneathImplicitTemplateInstantiation = true;
  }
  bool Result = RecursiveASTVisitor<
      IndexerASTVisitor>::TraverseVarTemplateSpecializationDecl(TD);
  Job->UnderneathImplicitTemplateInstantiation = UITI;
  return Result;
}

bool IndexerASTVisitor::TraverseClassTemplatePartialSpecializationDecl(
    clang::ClassTemplatePartialSpecializationDecl *TD) {
  // Implicit partial specializations don't happen, so we don't need
  // to consider changing the Job->RangeContext stack.
  Job->TypeContext.push_back(TD->getTemplateParameters());
  bool Result = RecursiveASTVisitor<
      IndexerASTVisitor>::TraverseClassTemplatePartialSpecializationDecl(TD);
  Job->TypeContext.pop_back();
  return Result;
}

bool IndexerASTVisitor::TraverseVarTemplateDecl(clang::VarTemplateDecl *TD) {
  Job->TypeContext.push_back(TD->getTemplateParameters());
  if (!TraverseDecl(TD->getTemplatedDecl())) {
    Job->TypeContext.pop_back();
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
              Job->TypeContext.pop_back();
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
  Job->TypeContext.pop_back();
  return true;
}

bool IndexerASTVisitor::TraverseVarTemplatePartialSpecializationDecl(
    clang::VarTemplatePartialSpecializationDecl *TD) {
  // Implicit partial specializations don't happen, so we don't need
  // to consider changing the Job->RangeContext stack.
  Job->TypeContext.push_back(TD->getTemplateParameters());
  bool Result = RecursiveASTVisitor<
      IndexerASTVisitor>::TraverseVarTemplatePartialSpecializationDecl(TD);
  Job->TypeContext.pop_back();
  return Result;
}

bool IndexerASTVisitor::TraverseTypeAliasTemplateDecl(
    clang::TypeAliasTemplateDecl *TATD) {
  Job->TypeContext.push_back(TATD->getTemplateParameters());
  TraverseDecl(TATD->getTemplatedDecl());
  Job->TypeContext.pop_back();
  return true;
}

bool IndexerASTVisitor::TraverseFunctionDecl(clang::FunctionDecl *FD) {
  bool UITI = Job->UnderneathImplicitTemplateInstantiation;
  if (const auto *MSI = FD->getMemberSpecializationInfo()) {
    // The definitions of class template member functions are not necessarily
    // dominated by the class template definition.
    if (!MSI->isExplicitSpecialization()) {
      Job->UnderneathImplicitTemplateInstantiation = true;
    }
  } else if (const auto *FSI = FD->getTemplateSpecializationInfo()) {
    if (!FSI->isExplicitInstantiationOrSpecialization()) {
      Job->UnderneathImplicitTemplateInstantiation = true;
    }
  }
  bool Result =
      RecursiveASTVisitor<IndexerASTVisitor>::TraverseFunctionDecl(FD);
  Job->UnderneathImplicitTemplateInstantiation = UITI;
  return Result;
}

bool IndexerASTVisitor::TraverseFunctionTemplateDecl(
    clang::FunctionTemplateDecl *FTD) {
  Job->TypeContext.push_back(FTD->getTemplateParameters());
  // We traverse the template parameter list when we visit the FunctionDecl.
  TraverseDecl(FTD->getTemplatedDecl());
  Job->TypeContext.pop_back();
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

MaybeFew<GraphObserver::Range> IndexerASTVisitor::ExplicitRangeInCurrentContext(
    const clang::SourceRange &SR) {
  if (!SR.getBegin().isValid()) {
    return None();
  }
  if (!Job->RangeContext.empty() &&
      !FLAGS_experimental_alias_template_instantiations) {
    return GraphObserver::Range(SR, Job->RangeContext.back());
  } else {
    return GraphObserver::Range(SR, Observer.getClaimTokenForRange(SR));
  }
}

MaybeFew<GraphObserver::Range> IndexerASTVisitor::RangeInCurrentContext(
    const MaybeFew<GraphObserver::NodeId> &Id, const clang::SourceRange &SR) {
  if (auto &PrimaryId = Id) {
    return GraphObserver::Range(PrimaryId.primary());
  }
  return ExplicitRangeInCurrentContext(SR);
}

MaybeFew<GraphObserver::Range> IndexerASTVisitor::RangeInCurrentContext(
    bool implicit, const GraphObserver::NodeId &Id,
    const clang::SourceRange &SR) {
  return implicit ? GraphObserver::Range(Id)
                  : ExplicitRangeInCurrentContext(SR);
}

GraphObserver::NodeId IndexerASTVisitor::RecordGenericClass(
    const ObjCInterfaceDecl *IDecl, const ObjCTypeParamList *TPL,
    const GraphObserver::NodeId &BodyId) {
  auto AbsId = BuildNodeIdForDecl(IDecl);
  Observer.recordAbsNode(AbsId);
  Observer.recordChildOfEdge(BodyId, AbsId);

  for (const ObjCTypeParamDecl *TP : *TPL) {
    GraphObserver::NodeId TypeParamId = BuildNodeIdForDecl(TP);
    // We record the range, name, absvar identity, and variance when we visit
    // the type param decl. Here, we only want to record the information that is
    // specific to this context, which is how it interacts with the generic type
    // (BodyID). See VisitObjCTypeParamDecl for where we record this other
    // information.
    Observer.recordParamEdge(AbsId, TP->getIndex(), TypeParamId);
  }
  return AbsId;
}

template <typename TemplateDeclish>
GraphObserver::NodeId IndexerASTVisitor::RecordTemplate(
    const TemplateDeclish *Decl, const GraphObserver::NodeId &BodyDeclNode) {
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
    Observer.recordParamEdge(DeclNode, ParamIndex, ParamId);
  }
  return DeclNode;
}

bool IndexerASTVisitor::VisitRecordDecl(const clang::RecordDecl *Decl) {
  if (SkipAliasedDecl(Decl)) {
    return true;
  }
  if (Decl->isInjectedClassName()) {
    // We can't ignore this in ::Traverse* and still make use of the code that
    // traverses template instantiations (since that functionality is marked
    // private), so we have to ignore it here.
    return true;
  }
  if (Decl->isEmbeddedInDeclarator() && Decl->getDefinition() != Decl) {
    return true;
  }

  auto Marks = MarkedSources.Generate(Decl);
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
    Marks.set_implicit(Job->UnderneathImplicitTemplateInstantiation ||
                       !CTSD->isExplicitInstantiationOrSpecialization());
    // If this is a partial specialization, we've already recorded the newly
    // abstracted parameters above. We can now record the type arguments passed
    // to the template we're specializing. Synthesize the type we need.
    std::vector<GraphObserver::NodeId> NIDS;
    std::vector<const GraphObserver::NodeId *> NIDPS;
    auto PrimaryOrPartial = CTSD->getSpecializedTemplateOrPartial();
    if (BuildTemplateArgumentList(
            ArgsAsWritten ? llvm::makeArrayRef(ArgsAsWritten->getTemplateArgs(),
                                               ArgsAsWritten->NumTemplateArgs)
                          : Optional<ArrayRef<TemplateArgumentLoc>>(),
            CTSD->getTemplateArgs().asArray(), NIDS, NIDPS)) {
      if (auto SpecializedNode = BuildNodeIdForTemplateName(
              clang::TemplateName(CTSD->getSpecializedTemplate()),
              CTSD->getPointOfInstantiation())) {
        if (PrimaryOrPartial.is<clang::ClassTemplateDecl *>() &&
            !isa<const clang::ClassTemplatePartialSpecializationDecl>(Decl)) {
          // This is both an instance and a specialization of the primary
          // template. We use the same arguments list for both.
          Observer.recordInstEdge(
              DeclNode,
              Observer.recordTappNode(SpecializedNode.primary(), NIDPS,
                                      ArgsAsWritten
                                          ? ArgsAsWritten->NumTemplateArgs
                                          : NIDPS.size()),
              GraphObserver::Confidence::NonSpeculative);
        }
        Observer.recordSpecEdge(
            DeclNode,
            Observer.recordTappNode(
                SpecializedNode.primary(), NIDPS,
                ArgsAsWritten ? ArgsAsWritten->NumTemplateArgs : NIDPS.size()),
            GraphObserver::Confidence::NonSpeculative);
      }
    }
    if (const auto *Partial =
            PrimaryOrPartial
                .dyn_cast<clang::ClassTemplatePartialSpecializationDecl *>()) {
      if (BuildTemplateArgumentList(
              llvm::None, CTSD->getTemplateInstantiationArgs().asArray(), NIDS,
              NIDPS)) {
        Observer.recordInstEdge(
            DeclNode,
            Observer.recordTappNode(BuildNodeIdForDecl(Partial), NIDPS,
                                    NIDPS.size()),
            GraphObserver::Confidence::NonSpeculative);
      }
    }
  }
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode);
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
                              GraphObserver::Completeness::Incomplete, None());
    Observer.recordMarkedSource(DeclNode, Marks.GenerateMarkedSource(DeclNode));
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
                            GraphObserver::Completeness::Definition, None());
  Observer.recordMarkedSource(DeclNode, Marks.GenerateMarkedSource(DeclNode));
  return true;
}

bool IndexerASTVisitor::VisitFunctionDecl(clang::FunctionDecl *Decl) {
  if (SkipAliasedDecl(Decl)) {
    return true;
  }
  auto Marks = MarkedSources.Generate(Decl);
  GraphObserver::NodeId InnerNode(Observer.getDefaultClaimToken(), "");
  GraphObserver::NodeId OuterNode(Observer.getDefaultClaimToken(), "");
  // There are five flavors of function (see TemplateOrSpecialization in
  // FunctionDecl).
  const clang::TemplateArgumentLoc *ArgsAsWritten = nullptr;
  unsigned NumArgsAsWritten = 0;
  const clang::TemplateArgumentList *Args = nullptr;
  std::vector<std::pair<clang::TemplateName, clang::SourceLocation>> TNs;
  bool TNsAreSpeculative = false;
  bool IsImplicit = false;
  if (auto *FTD = Decl->getDescribedFunctionTemplate()) {
    // Function template (inc. overloads)
    InnerNode = BuildNodeIdForDecl(Decl, 0);
    OuterNode = RecordTemplate(FTD, InnerNode);
  } else if (auto *MSI = Decl->getMemberSpecializationInfo()) {
    // Here, template variables are bound by our enclosing context.
    // For example:
    // `template <typename T> struct S { int f() { return 0; } };`
    // `int q() { S<int> s; return s.f(); }`
    // We're going to be a little loose with `instantiates` here. To make
    // queries consistent, we'll use a nullary tapp() to refer to the
    // instantiated object underneath a template. It may be useful to also
    // add the type variables involve in this instantiation's particular
    // parent, but we don't currently do so. (T230)
    IsImplicit = !MSI->isExplicitSpecialization();
    InnerNode = BuildNodeIdForDecl(Decl);
    OuterNode = InnerNode;
    Observer.recordInstEdge(
        OuterNode,
        Observer.recordTappNode(BuildNodeIdForDecl(MSI->getInstantiatedFrom()),
                                {}, 0),
        GraphObserver::Confidence::NonSpeculative);
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
    IsImplicit = !FTSI->isExplicitSpecialization();
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
  Marks.set_implicit(Job->UnderneathImplicitTemplateInstantiation ||
                     IsImplicit);
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
              Observer.recordTappNode(SpecializedNode.primary(), NIDPS,
                                      NumArgsAsWritten),
              Confidence);
          Observer.recordSpecEdge(
              OuterNode,
              Observer.recordTappNode(SpecializedNode.primary(), NIDPS,
                                      NumArgsAsWritten),
              Confidence);
        }
      }
    }
  }
  SourceLocation DeclLoc = Decl->getLocation();
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  if (!DeclLoc.isMacroID() && Decl->isThisDeclarationADefinition() &&
      Decl->hasBody()) {
    Marks.set_marked_source_end(Decl->getBody()->getSourceRange().getBegin());
  } else {
    Marks.set_marked_source_end(GetLocForEndOfToken(
        *Observer.getSourceManager(), *Observer.getLangOptions(),
        Decl->getSourceRange().getEnd()));
  }
  Marks.set_name_range(NameRange);
  auto NameRangeInContext(
      RangeInCurrentContext(Decl->isImplicit(), OuterNode, NameRange));
  MaybeRecordDefinitionRange(NameRangeInContext, OuterNode);
  bool IsFunctionDefinition = IsDefinition(Decl);
  unsigned ParamNumber = 0;
  for (const auto *Param : Decl->parameters()) {
    ConnectParam(Decl, InnerNode, IsFunctionDefinition, ParamNumber++, Param,
                 IsImplicit);
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
    // OO_Call, OO_Subscript, and OO_Equal must be member functions.
    // The dyn_cast to CXXMethodDecl above is therefore not dropping
    // (impossible) free function incarnations of these operators from
    // consideration in the following.
    if (MF->size_overridden_methods() != 0) {
      for (auto O = MF->begin_overridden_methods(),
                E = MF->end_overridden_methods();
           O != E; ++O) {
        Observer.recordOverridesEdge(InnerNode, BuildNodeIdForDecl(*O));
      }
      MapOverrideRoots(MF, [&](const CXXMethodDecl *R) {
        Observer.recordOverridesRootEdge(InnerNode, BuildNodeIdForDecl(R));
      });
    }
  }

  AddChildOfEdgeToDeclContext(Decl, OuterNode);
  GraphObserver::FunctionSubkind Subkind = GraphObserver::FunctionSubkind::None;
  if (const auto *CC = dyn_cast<CXXConstructorDecl>(Decl)) {
    Subkind = GraphObserver::FunctionSubkind::Constructor;
    size_t InitNumber = 0;
    for (const auto *Init : CC->inits()) {
      ++InitNumber;

      // Draw ref edge for the field we are initializing if we're doing so
      // explicitly.
      if (auto M = Init->getMember()) {
        if (Init->isWritten()) {
          // This range is too large, if we have `A() : b_(10)`, it returns the
          // range for b_(10), not b_.
          auto MemberSR = Init->getSourceRange();
          // If we are in a valid file, we can be more precise with the range
          // for the variable we are initializing.
          const SourceLocation &Loc = Init->getMemberLocation();
          if (Loc.isValid() && Loc.isFileID()) {
            MemberSR = RangeForSingleTokenFromSourceLocation(
                *Observer.getSourceManager(), *Observer.getLangOptions(), Loc);
          }
          if (auto RCC = ExplicitRangeInCurrentContext(MemberSR)) {
            const auto &ID = BuildNodeIdForRefToDecl(M);
            Observer.recordDeclUseLocation(
                RCC.primary(), ID, GraphObserver::Claimability::Claimable);
          }
        }
      }

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
  if (!IsFunctionDefinition && Decl->getBuiltinID() == 0) {
    Observer.recordFunctionNode(
        InnerNode, GraphObserver::Completeness::Incomplete, Subkind, None());
    Observer.recordMarkedSource(OuterNode,
                                Marks.GenerateMarkedSource(OuterNode));
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
  Observer.recordFunctionNode(
      InnerNode, GraphObserver::Completeness::Definition, Subkind, None());
  Observer.recordMarkedSource(OuterNode, Marks.GenerateMarkedSource(OuterNode));
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
    auto Marks = MarkedSources.Generate(Decl);
    Marks.set_implicit(Job->UnderneathImplicitTemplateInstantiation);
    GraphObserver::NameId AliasNameId(BuildNameIdForDecl(Decl));
    return Observer.recordTypeAliasNode(
        AliasNameId, AliasedTypeId.primary(),
        BuildNodeIdForType(FollowAliasChain(Decl)),
        Marks.GenerateMarkedSource(Observer.nodeIdForTypeAliasNode(
            AliasNameId, AliasedTypeId.primary())));
  }
  return None();
}

bool IndexerASTVisitor::VisitObjCTypeParamDecl(
    const clang::ObjCTypeParamDecl *Decl) {
  GraphObserver::NodeId TypeParamId = BuildNodeIdForDecl(Decl);
  Observer.recordAbsVarNode(TypeParamId);
  SourceRange TypeSR = RangeForNameOfDeclaration(Decl);
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), TypeParamId, TypeSR),
      TypeParamId);
  GraphObserver::Variance V;
  switch (Decl->getVariance()) {
    case clang::ObjCTypeParamVariance::Contravariant:
      V = GraphObserver::Variance::Contravariant;
      break;
    case clang::ObjCTypeParamVariance::Covariant:
      V = GraphObserver::Variance::Covariant;
      break;
    case clang::ObjCTypeParamVariance::Invariant:
      V = GraphObserver::Variance::Invariant;
      break;
  }
  Observer.recordVariance(TypeParamId, V);

  // If the type has an explicit bound, getTypeSourceInfo will get the bound
  // for us. If there is not explicit bound, it will return id.
  auto BoundInfo = Decl->getTypeSourceInfo();

  if (auto Type = BuildNodeIdForType(BoundInfo->getTypeLoc(),
                                     BoundInfo->getType(), EmitRanges::Yes)) {
    if (Decl->hasExplicitBound()) {
      Observer.recordUpperBoundEdge(TypeParamId, Type.primary());
    } else {
      Observer.recordTypeEdge(TypeParamId, Type.primary());
    }
  }

  return true;
}

bool IndexerASTVisitor::VisitTypedefDecl(const clang::TypedefDecl *Decl) {
  return VisitCTypedef(Decl);
}

bool IndexerASTVisitor::VisitTypeAliasDecl(const clang::TypeAliasDecl *Decl) {
  return VisitCTypedef(Decl);
}

bool IndexerASTVisitor::VisitCTypedef(const clang::TypedefNameDecl *Decl) {
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

GraphObserver::NameId::NameEqClass IndexerASTVisitor::BuildNameEqClassForDecl(
    const clang::Decl *D) {
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
  } else if (isa<clang::ObjCContainerDecl>(D)) {
    // In the future we might want to do something different for each of the
    // important subclasses: clang::ObjCInterfaceDecl,
    // clang::ObjCImplementationDecl, clang::ObjCProtocolDecl,
    // clang::ObjCCategoryDecl, and clang::ObjCCategoryImplDecl.
    return GraphObserver::NameId::NameEqClass::Class;
  }
  return GraphObserver::NameId::NameEqClass::None;
}

// TODO(zarko): Can this logic be shared with BuildNameIdForDecl?
MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::GetDeclChildOf(
    const clang::Decl *Decl) {
  clang::ast_type_traits::DynTypedNode CurrentNode =
      clang::ast_type_traits::DynTypedNode::create(*Decl);
  const clang::Decl *CurrentNodeAsDecl;
  while (!(CurrentNodeAsDecl = CurrentNode.get<clang::Decl>()) ||
         !isa<clang::TranslationUnitDecl>(CurrentNodeAsDecl)) {
    IndexedParent *IP = getIndexedParent(CurrentNode);
    if (IP == nullptr) {
      break;
    }
    // We would rather name 'template <etc> class C' as C, not C::C, but
    // we also want to be able to give useful names to templates when they're
    // explicitly requested. Therefore:
    if (CurrentNodeAsDecl == Decl ||
        (CurrentNodeAsDecl && isa<ClassTemplateDecl>(CurrentNodeAsDecl))) {
      CurrentNode = IP->Parent;
      continue;
    }
    if (CurrentNodeAsDecl) {
      if (const NamedDecl *ND = dyn_cast<NamedDecl>(CurrentNodeAsDecl)) {
        return BuildNodeIdForDecl(ND);
      }
    }
    CurrentNode = IP->Parent;
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
#define OVERLOADED_OPERATOR(Name, Spelling, Token, Unary, Binary, MemberOnly) \
  case OO_##Name:                                                             \
    Ostream << "OO#" << #Name;                                                \
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
    } else if (isObjCSelector(Name)) {
      Ostream << HashToString(SemanticHash(Name.getObjCSelector()));
    } else {
      // Other NamedDecls-sans-names are given parent-dependent names.
      return false;
    }
  }
  return true;
}

GraphObserver::NameId IndexerASTVisitor::BuildNameIdForDecl(
    const clang::Decl *Decl) {
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
    IndexedParent *IP = getIndexedParent(CurrentNode);
    if (IP == nullptr) {
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
                for (const auto *P : FD->parameters()) {
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
    // We would rather name 'template <etc> class C' as C, not C::C, but
    // we also want to be able to give useful names to templates when they're
    // explicitly requested. Therefore:
    if (MissingSeparator && CurrentNodeAsDecl &&
        isa<ClassTemplateDecl>(CurrentNodeAsDecl)) {
      CurrentNode = IP->Parent;
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
          Ostream << IP->Index;
        }
      } else {
        // If there's no good name for this Decl, name it after its child
        // index wrt its parent node.
        Ostream << IP->Index;
      }
    } else if (auto *S = CurrentNode.get<clang::Stmt>()) {
      // This is a Stmt--we can name it by its index wrt its parent node.
      Ostream << IP->Index;
    }
    CurrentNode = IP->Parent;
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
uint64_t IndexerASTVisitor::SemanticHashTemplateDeclish(
    const TemplateDeclish *Decl) {
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
      CHECK(IgnoreUnimplemented)
          << "SemanticHash(SubstTemplateTemplateParmPack)";
      return 0;
    default:
      LOG(FATAL) << "Unexpected TemplateName Kind";
  }
  return 0;
}

uint64_t IndexerASTVisitor::SemanticHash(const clang::TemplateArgument &TA) {
  switch (TA.getKind()) {
    case TemplateArgument::Null:
      return 0x1010101001010101LL;  // Arbitrary constant for H(Null).
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
  if (!Ins.second) return Ins.first->second;

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

uint64_t IndexerASTVisitor::SemanticHash(
    const clang::TemplateArgumentList *RD) {
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

uint64_t IndexerASTVisitor::SemanticHash(const clang::Selector &S) {
  return std::hash<std::string>()(S.getAsString());
}

GraphObserver::NodeId IndexerASTVisitor::BuildNodeIdForDecl(
    const clang::Decl *Decl, unsigned Index) {
  GraphObserver::NodeId BaseId(BuildNodeIdForDecl(Decl));
  return GraphObserver::NodeId(
      BaseId.getToken(), BaseId.getRawIdentity() + "." + std::to_string(Index));
}

MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::BuildNodeIdForImplicitStmt(
    const clang::Stmt *Stmt) {
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
      // If this is an implicit variable declaration, we assume that it is one
      // of the implicit declarations attached to a range for loop. We ignore
      // its implicitness, which lets us associate a source location with the
      // implicit references to 'begin', 'end' and operators.
      if (CurrentNodeAsDecl->isImplicit() && !isa<VarDecl>(CurrentNodeAsDecl)) {
        break;
      } else if (isa<clang::TranslationUnitDecl>(CurrentNodeAsDecl)) {
        CHECK(!CurrentNodeAsDecl->isImplicit());
        return None();
      }
    }
    IndexedParent *IP = getIndexedParent(CurrentNode);
    if (IP == nullptr) {
      break;
    }
    StmtPath.push_back(IP->Index);
    CurrentNode = IP->Parent;
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

GraphObserver::NodeId IndexerASTVisitor::BuildNodeIdForDecl(
    const clang::Decl *Decl) {
  // We can't assume that no two nodes of the same Kind can appear
  // simultaneously at the same SourceLocation: witness implicit overloaded
  // operator=. We rely on names and types to disambiguate them.
  // NodeIds must be stable across analysis runs with the same input data.
  // Some NodeIds are stable in the face of changes to that data, such as
  // the IDs given to class definitions (in part because of the language rules).

  if (FLAGS_experimental_alias_template_instantiations) {
    Decl = FindSpecializedTemplate(Decl);
  }

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

  if (const auto *BTD = dyn_cast<BuiltinTemplateDecl>(Decl)) {
    Ostream << "#builtin";
    GraphObserver::NodeId Id(Observer.getClaimTokenForBuiltin(), Ostream.str());
    DeclToNodeId.insert(std::make_pair(Decl, Id));
    return Id;
  }

  const TypedefNameDecl *TND;
  if ((TND = dyn_cast<TypedefNameDecl>(Decl)) &&
      !isa<ObjCTypeParamDecl>(Decl)) {
    // There's a special way to name type aliases but we want to handle type
    // parameters for Objective-C as "normal" named decls.
    if (auto TypedefNameId = BuildNodeIdForTypedefNameDecl(TND)) {
      DeclToNodeId.insert(std::make_pair(Decl, TypedefNameId.primary()));
      return TypedefNameId.primary();
    }
  } else if (const auto *NS = dyn_cast<NamespaceDecl>(Decl)) {
    // Namespaces are named according to their NameIDs.
    Ostream << "#namespace";
    GraphObserver::NodeId Id(
        NS->isAnonymousNamespace()
            ? Observer.getAnonymousNamespaceClaimToken(NS->getLocation())
            : Observer.getNamespaceClaimToken(NS->getLocation()),
        Ostream.str());
    DeclToNodeId.insert(std::make_pair(Decl, Id));
    return Id;
  }

  // Disambiguate nodes underneath template instances.
  clang::ast_type_traits::DynTypedNode CurrentNode =
      clang::ast_type_traits::DynTypedNode::create(*Decl);
  const clang::Decl *CurrentNodeAsDecl;
  while (!(CurrentNodeAsDecl = CurrentNode.get<clang::Decl>()) ||
         !isa<clang::TranslationUnitDecl>(CurrentNodeAsDecl)) {
    IndexedParent *IP = getIndexedParent(CurrentNode);
    if (IP == nullptr) {
      break;
    }
    CurrentNode = IP->Parent;
    if (!CurrentNodeAsDecl) {
      continue;
    }
    if (const auto *TD = dyn_cast<TemplateDecl>(CurrentNodeAsDecl)) {
      // Disambiguate type abstraction IDs from abstracted type IDs.
      if (CurrentNodeAsDecl != Decl) {
        Ostream << "#";
      }
    }
    if (!FLAGS_experimental_alias_template_instantiations) {
      if (const auto *CTSD =
              dyn_cast<ClassTemplateSpecializationDecl>(CurrentNodeAsDecl)) {
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
          Ostream << "#"
                  << HashToString(
                         SemanticHash(&CTSD->getTemplateInstantiationArgs()));
        }
      } else if (const auto *FD = dyn_cast<FunctionDecl>(CurrentNodeAsDecl)) {
        Ostream << "#"
                << HashToString(
                       SemanticHash(QualType(FD->getFunctionType(), 0)));
        if (const auto *TemplateArgs = FD->getTemplateSpecializationArgs()) {
          if (CurrentNodeAsDecl != Decl) {
            Ostream << "#" << BuildNodeIdForDecl(FD);
            break;
          } else {
            Ostream << "#" << HashToString(SemanticHash(TemplateArgs));
          }
        } else if (const auto *MSI = FD->getMemberSpecializationInfo()) {
          if (const auto *DC =
                  dyn_cast<const class Decl>(FD->getDeclContext())) {
            Ostream << "#" << BuildNodeIdForDecl(DC);
            break;
          }
        }
      } else if (const auto *VD = dyn_cast<VarTemplateSpecializationDecl>(
                     CurrentNodeAsDecl)) {
        if (VD->isImplicit()) {
          if (CurrentNodeAsDecl != Decl) {
            Ostream << "#" << BuildNodeIdForDecl(VD);
            break;
          } else {
            Ostream << "#"
                    << HashToString(
                           SemanticHash(&VD->getTemplateInstantiationArgs()));
          }
        }
      } else if (const auto *VD = dyn_cast<VarDecl>(CurrentNodeAsDecl)) {
        if (const auto *MSI = VD->getMemberSpecializationInfo()) {
          if (const auto *DC =
                  dyn_cast<const class Decl>(VD->getDeclContext())) {
            Ostream << "#" << BuildNodeIdForDecl(DC);
            break;
          }
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

// There aren't too many types in C++, as it turns out. See
// clang/AST/TypeNodes.def.

#define UNSUPPORTED_CLANG_TYPE(t)                  \
  case TypeLoc::t:                                 \
    if (IgnoreUnimplemented) {                     \
      return None();                               \
    } else {                                       \
      LOG(FATAL) << "TypeLoc::" #t " unsupported"; \
    }                                              \
    break

MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::ApplyBuiltinTypeConstructor(
    const char *BuiltinName, const MaybeFew<GraphObserver::NodeId> &Param) {
  GraphObserver::NodeId TyconID(Observer.getNodeIdForBuiltinType(BuiltinName));
  return Param.Map<GraphObserver::NodeId>(
      [this, &TyconID, &BuiltinName](const GraphObserver::NodeId &Elt) {
        return Observer.recordTappNode(TyconID, {&Elt}, 1);
      });
}

MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::BuildNodeIdForTemplateName(
    const clang::TemplateName &Name, const clang::SourceLocation L) {
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
      } else if (const auto *BTD = dyn_cast<BuiltinTemplateDecl>(TD)) {
        return BuildNodeIdForDecl(BTD);
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
  Ostream << "#nns";  // Nested name specifier.
  clang::SourceRange NNSRange(InNNSLoc.getBeginLoc(), InNNSLoc.getEndLoc());
  Ostream << "@";
  if (auto Range = ExplicitRangeInCurrentContext(NNSRange)) {
    Observer.AppendRangeToStream(Ostream, Range.primary());
  } else {
    Ostream << "invalid";
  }
  Ostream << "@";
  if (auto RCC =
          ExplicitRangeInCurrentContext(RangeForASTEntityFromSourceLocation(
              *Observer.getSourceManager(), *Observer.getLangOptions(),
              IdLoc))) {
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
      Observer.recordLookupNode(IdOut,
                                Id.getAsIdentifierInfo()->getNameStart());
      break;
    case clang::DeclarationName::CXXConstructorName:
      Observer.recordLookupNode(IdOut, "#ctor");
      break;
    case clang::DeclarationName::CXXDestructorName:
      Observer.recordLookupNode(IdOut, "#dtor");
      break;
// TODO(zarko): Fill in the remaining relevant DeclarationName cases.
#define UNEXPECTED_DECLARATION_NAME_KIND(kind)                         \
  case clang::DeclarationName::kind:                                   \
    CHECK(IgnoreUnimplemented) << "Unexpected DeclaraionName::" #kind; \
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
    if (auto RCC =
            ExplicitRangeInCurrentContext(RangeForASTEntityFromSourceLocation(
                *Observer.getSourceManager(), *Observer.getLangOptions(),
                IdLoc))) {
      Observer.recordDeclUseLocation(RCC.primary(), IdOut);
    }
  }
  return IdOut;
}

MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::BuildNodeIdForExpr(
    const clang::Expr *Expr, EmitRanges ER) {
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
      RangeForASTEntityFromSourceLocation(*Observer.getSourceManager(),
                                          *Observer.getLangOptions(),
                                          Expr->getExprLoc()));
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
  for (unsigned D = 0; D < Job->TypeContext.size(); ++D) {
    llvm::errs() << "  Depth " << D << " ---- \n";
    for (unsigned I = 0; I < Job->TypeContext[D]->size(); ++I) {
      llvm::errs() << "    Index " << I << " ";
      Job->TypeContext[D]->getParam(I)->dump();
      llvm::errs() << "\n";
    }
  }
}

MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::BuildNodeIdForType(
    const clang::TypeLoc &Type, EmitRanges EmitRanges) {
  return BuildNodeIdForType(Type, Type.getTypePtr(), EmitRanges);
}

MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::BuildNodeIdForType(
    const clang::TypeLoc &Type, const clang::QualType &QT,
    EmitRanges EmitRanges) {
  return BuildNodeIdForType(Type, QT.getTypePtr(), EmitRanges);
}

MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::BuildNodeIdForType(
    const clang::QualType &QT) {
  CHECK(!QT.isNull());
  TypeSourceInfo *TSI = Context.getTrivialTypeSourceInfo(QT, SourceLocation());
  return BuildNodeIdForType(TSI->getTypeLoc(), EmitRanges::No);
}

MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::BuildNodeIdForType(
    const clang::TypeLoc &TypeLoc, const clang::Type *PT,
    const EmitRanges EmitRanges) {
  return BuildNodeIdForType(TypeLoc, PT, EmitRanges, TypeLoc.getSourceRange());
}

MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::BuildNodeIdForType(
    const clang::TypeLoc &TypeLoc, const clang::Type *PT,
    const EmitRanges EmitRanges, SourceRange SR) {
  // If we have an attributed type, treat it as the raw type, but provide the
  // source range of the attributed type. This allows us to connect a _Nonnull
  // type back to the underlying type's definition. For example:
  // `@property Data * _Nullable var;` should connect back to the type Data *,
  // not Data * _Nullable;
  if (TypeLoc.getTypeLocClass() == TypeLoc::Attributed) {
    // Sometimes the TypeLoc is Attributed, but the type is actually
    // TypedefType.
    if (const auto *AT = dyn_cast<AttributedType>(PT)) {
      const auto &ATL = TypeLoc.castAs<AttributedTypeLoc>();
      return BuildNodeIdForType(ATL.getModifiedLoc(),
                                AT->getModifiedType().getTypePtr(), EmitRanges,
                                SR);
    }
  }
  MaybeFew<GraphObserver::NodeId> ID;
  const QualType QT = TypeLoc.getType();
  int64_t Key = ComputeKeyFromQualType(Context, QT, PT);
  const auto &Prev = TypeNodes.find(Key);
  bool TypeAlreadyBuilt = false;
  GraphObserver::Claimability Claimability =
      GraphObserver::Claimability::Claimable;
  if (Prev != TypeNodes.end()) {
    // If we're not trying to emit edges for constituent types, or if there's
    // no chance for us to do so because we lack source location information,
    // finish early. Unless we are an ObjCObjectPointer, then we always want to
    // visit because we may need to record information for implemented
    // protocols.
    if (TypeLoc.getTypeLocClass() != TypeLoc::ObjCObjectPointer &&
        (EmitRanges != IndexerASTVisitor::EmitRanges::Yes ||
         !(SR.isValid() && SR.getBegin().isFileID()))) {
      ID = Prev->second;
      return ID;
    }
    TypeAlreadyBuilt = true;
  }
  // We may turn off emitting ranges for particular types, but we still want
  // to emit ranges recursively. For example, for the type T*, we don't want to
  // emit a range for [T*], but we do want to emit one for [T]*.
  auto InEmitRanges = EmitRanges;

  // We only care about leaves in the type hierarchy (eg, we shouldn't match
  // on Reference, but instead on LValueReference or RValueReference).
  switch (TypeLoc.getTypeLocClass()) {
    case TypeLoc::Qualified: {
      const auto &T = TypeLoc.castAs<QualifiedTypeLoc>();
      InEmitRanges = IndexerASTVisitor::EmitRanges::No;
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
    case TypeLoc::Builtin: {  // Leaf.
      const auto &T = TypeLoc.castAs<BuiltinTypeLoc>();
      if (!FLAGS_emit_anchors_on_builtins) {
        InEmitRanges = IndexerASTVisitor::EmitRanges::No;
      }
      if (TypeAlreadyBuilt) {
        break;
      }
      ID = Observer.getNodeIdForBuiltinType(T.getTypePtr()->getName(
          clang::PrintingPolicy(*Observer.getLangOptions())));
    } break;
      UNSUPPORTED_CLANG_TYPE(Complex);
    case TypeLoc::Pointer: {
      const auto &T = TypeLoc.castAs<PointerTypeLoc>();
      const auto *DT = dyn_cast<PointerType>(PT);
      InEmitRanges = IndexerASTVisitor::EmitRanges::No;
      auto PointeeID(BuildNodeIdForType(T.getPointeeLoc(),
                                        DT ? DT->getPointeeType().getTypePtr()
                                           : T.getPointeeLoc().getTypePtr(),
                                        EmitRanges));
      if (!PointeeID) {
        return PointeeID;
      }
      if (TypeAlreadyBuilt) {
        break;
      }
      ID = ApplyBuiltinTypeConstructor("ptr", PointeeID);
    } break;
    case TypeLoc::LValueReference: {
      const auto &T = TypeLoc.castAs<LValueReferenceTypeLoc>();
      const auto *DT = dyn_cast<LValueReferenceType>(PT);
      InEmitRanges = IndexerASTVisitor::EmitRanges::No;
      auto ReferentID(BuildNodeIdForType(T.getPointeeLoc(),
                                         DT ? DT->getPointeeType().getTypePtr()
                                            : T.getPointeeLoc().getTypePtr(),
                                         EmitRanges));
      if (!ReferentID) {
        return ReferentID;
      }
      if (TypeAlreadyBuilt) {
        break;
      }
      ID = ApplyBuiltinTypeConstructor("lvr", ReferentID);
    } break;
    case TypeLoc::RValueReference: {
      const auto &T = TypeLoc.castAs<RValueReferenceTypeLoc>();
      const auto *DT = dyn_cast<RValueReferenceType>(PT);
      InEmitRanges = IndexerASTVisitor::EmitRanges::No;
      auto ReferentID(BuildNodeIdForType(T.getPointeeLoc(),
                                         DT ? DT->getPointeeType().getTypePtr()
                                            : T.getPointeeLoc().getTypePtr(),
                                         EmitRanges));
      if (!ReferentID) {
        return ReferentID;
      }
      if (TypeAlreadyBuilt) {
        break;
      }
      ID = ApplyBuiltinTypeConstructor("rvr", ReferentID);
    } break;
      UNSUPPORTED_CLANG_TYPE(MemberPointer);
    case TypeLoc::ConstantArray: {
      const auto &T = TypeLoc.castAs<ConstantArrayTypeLoc>();
      const auto *DT = dyn_cast<ConstantArrayType>(PT);
      InEmitRanges = IndexerASTVisitor::EmitRanges::No;
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
      const auto &T = TypeLoc.castAs<IncompleteArrayTypeLoc>();
      const auto *DT = dyn_cast<IncompleteArrayType>(PT);
      InEmitRanges = IndexerASTVisitor::EmitRanges::No;
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
      const auto &T = TypeLoc.castAs<DependentSizedArrayTypeLoc>();
      const auto *DT = dyn_cast<DependentSizedArrayType>(PT);
      InEmitRanges = IndexerASTVisitor::EmitRanges::No;
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
            {{&ElementID.primary(), &ExpressionID.primary()}}, 2);
      } else {
        ID = ApplyBuiltinTypeConstructor("darr", ElementID);
      }
    } break;
      UNSUPPORTED_CLANG_TYPE(DependentSizedExtVector);
      UNSUPPORTED_CLANG_TYPE(Vector);
      UNSUPPORTED_CLANG_TYPE(ExtVector);
    case TypeLoc::FunctionProto: {
      const auto &T = TypeLoc.castAs<FunctionProtoTypeLoc>();
      const auto *FT = cast<clang::FunctionProtoType>(TypeLoc.getType());
      const auto *DT = dyn_cast<FunctionProtoType>(PT);
      InEmitRanges = IndexerASTVisitor::EmitRanges::No;
      std::vector<GraphObserver::NodeId> NodeIds;
      std::vector<const GraphObserver::NodeId *> NodeIdPtrs;
      auto ReturnType(BuildNodeIdForType(
          T.getReturnLoc(),
          DT ? DT->getReturnType().getTypePtr() : T.getReturnLoc().getTypePtr(),
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
                                     NodeIdPtrs, NodeIdPtrs.size());
      }
    } break;
    case TypeLoc::FunctionNoProto:
      InEmitRanges = IndexerASTVisitor::EmitRanges::No;
      if (!TypeAlreadyBuilt) {
        const auto &T = TypeLoc.castAs<FunctionNoProtoTypeLoc>();
        ID = Observer.getNodeIdForBuiltinType("knrfn");
      }
      break;
      UNSUPPORTED_CLANG_TYPE(UnresolvedUsing);
    case TypeLoc::Paren: {
      const auto &T = TypeLoc.castAs<ParenTypeLoc>();
      const auto *DT = dyn_cast<ParenType>(PT);
      InEmitRanges = IndexerASTVisitor::EmitRanges::No;
      ID = BuildNodeIdForType(
          T.getInnerLoc(),
          DT ? DT->getInnerType().getTypePtr() : T.getInnerLoc().getTypePtr(),
          EmitRanges);
    } break;
    case TypeLoc::Typedef: {
      // TODO(zarko): Return canonicalized versions as non-primary elements of
      // the MaybeFew.
      const auto &T = TypeLoc.castAs<TypedefTypeLoc>();
      GraphObserver::NameId AliasID(BuildNameIdForDecl(T.getTypedefNameDecl()));
      // We're retrieving the type of an alias here, so we shouldn't thread
      // through the deduced type.
      auto AliasedTypeID(BuildNodeIdForType(
          T.getTypedefNameDecl()->getTypeSourceInfo()->getTypeLoc(),
          IndexerASTVisitor::EmitRanges::No));
      if (!AliasedTypeID) {
        return AliasedTypeID;
      }
      auto Marks = MarkedSources.Generate(T.getTypedefNameDecl());
      ID = TypeAlreadyBuilt
               ? Observer.nodeIdForTypeAliasNode(AliasID,
                                                 AliasedTypeID.primary())
               : Observer.recordTypeAliasNode(
                     AliasID, AliasedTypeID.primary(),
                     BuildNodeIdForType(
                         FollowAliasChain(T.getTypedefNameDecl())),
                     Marks.GenerateMarkedSource(Observer.nodeIdForTypeAliasNode(
                         AliasID, AliasedTypeID.primary())));
    } break;
      UNSUPPORTED_CLANG_TYPE(Adjusted);
      UNSUPPORTED_CLANG_TYPE(Decayed);
      UNSUPPORTED_CLANG_TYPE(TypeOfExpr);
      UNSUPPORTED_CLANG_TYPE(TypeOf);
    case TypeLoc::Decltype:
      if (!TypeAlreadyBuilt) {
        const auto &T = TypeLoc.castAs<DecltypeTypeLoc>();
        const auto *DT = dyn_cast<DecltypeType>(PT);
        ID = BuildNodeIdForType(DT ? DT->getUnderlyingType()
                                   : T.getTypePtr()->getUnderlyingType());
      }
      break;
      UNSUPPORTED_CLANG_TYPE(UnaryTransform);
    case TypeLoc::Record: {  // Leaf.
      const auto &T = TypeLoc.castAs<RecordTypeLoc>();
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
        auto Parent = GetDeclChildOf(SpecializedTemplateDecl);
        auto Marks = MarkedSources.Generate(SpecializedTemplateDecl);
        GraphObserver::NodeId TemplateName =
            Claimability == GraphObserver::Claimability::Unclaimable
                ? BuildNodeIdForDecl(SpecializedTemplateDecl)
                : Observer.recordNominalTypeNode(
                      BuildNameIdForDecl(SpecializedTemplateDecl),
                      Marks.GenerateMarkedSource(
                          Observer.nodeIdForNominalTypeNode(
                              BuildNameIdForDecl(SpecializedTemplateDecl))),
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
          ID = Observer.recordTappNode(TemplateName, TemplateArgsPtrs,
                                       TemplateArgsPtrs.size());
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
            auto Parent = GetDeclChildOf(Decl);
            auto Marks = MarkedSources.Generate(Decl);
            auto DeclNameId = BuildNameIdForDecl(Decl);
            ID = Observer.recordNominalTypeNode(
                DeclNameId,
                Marks.GenerateMarkedSource(
                    Observer.nodeIdForNominalTypeNode(DeclNameId)),
                Parent ? &Parent.primary() : nullptr);
          }
        }
      }
    } break;
    case TypeLoc::Enum: {  // Leaf.
      const auto &T = TypeLoc.castAs<EnumTypeLoc>();
      EnumDecl *Decl = T.getDecl();
      if (!TypeAlreadyBuilt) {
        if (EnumDecl *Defn = Decl->getDefinition()) {
          Claimability = GraphObserver::Claimability::Unclaimable;
          ID = BuildNodeIdForDecl(Defn);
        } else {
          auto Parent = GetDeclChildOf(Decl);
          auto Marks = MarkedSources.Generate(Decl);
          auto DeclNameId = BuildNameIdForDecl(Decl);
          ID = Observer.recordNominalTypeNode(
              DeclNameId,
              Marks.GenerateMarkedSource(
                  Observer.nodeIdForNominalTypeNode(DeclNameId)),
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
      const auto &T = TypeLoc.castAs<ElaboratedTypeLoc>();
      const auto *DT = dyn_cast<ElaboratedType>(PT);
      ID = BuildNodeIdForType(T.getNamedTypeLoc(),
                              DT ? DT->getNamedType().getTypePtr()
                                 : T.getNamedTypeLoc().getTypePtr(),
                              EmitRanges);
      // Add anchors for parts of the NestedNameSpecifier.
      if (EmitRanges == IndexerASTVisitor::EmitRanges::Yes) {
        VisitNestedNameSpecifierLoc(T.getQualifierLoc());
      }
      // Don't link 'struct'.
      InEmitRanges = IndexerASTVisitor::EmitRanges::No;
      // TODO(zarko): Add an anchor for all the Elaborated type; otherwise decls
      // like `typedef B::C tdef;` will only anchor `C` instead of `B::C`.
    } break;
    // Either the `TemplateTypeParm` will link directly to a relevant
    // `TemplateTypeParmDecl` or (particularly in the case of canonicalized
    // types) we will find the Decl in the `Job->TypeContext` according to the
    // parameter's
    // depth and index.
    case TypeLoc::TemplateTypeParm: {  // Leaf.
      // Depths count from the outside-in; each Template*ParmDecl has only
      // one possible (depth, index).
      const auto *TypeParm = cast<TemplateTypeParmType>(TypeLoc.getTypePtr());
      const auto *TD = TypeParm->getDecl();
      if (!IgnoreUnimplemented) {
        // TODO(zarko): Remove sanity checks. If things go poorly here,
        // dump with DumpTypeContext(T->getDepth(), T->getIndex());
        CHECK_LT(TypeParm->getDepth(), Job->TypeContext.size())
            << "Decl for type parameter missing from context.";
        CHECK_LT(TypeParm->getIndex(),
                 Job->TypeContext[TypeParm->getDepth()]->size())
            << "Decl for type parameter missing at specified depth.";
        const auto *ND = Job->TypeContext[TypeParm->getDepth()]->getParam(
            TypeParm->getIndex());
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
      const auto &T = TypeLoc.castAs<SubstTemplateTypeParmTypeLoc>();
      const SubstTemplateTypeParmType *STTPT = T.getTypePtr();
      // TODO(zarko): Record both the replaced parameter and the replacement
      // type.
      CHECK(!STTPT->getReplacementType().isNull());
      ID = BuildNodeIdForType(STTPT->getReplacementType());
    } break;
      // "When a pack expansion in the source code contains multiple parameter
      // packs
      // and those parameter packs correspond to different levels of template
      // parameter lists, this type node is used to represent a template type
      // parameter pack from an outer level, which has already had its argument
      // pack
      // substituted but that still lives within a pack expansion that itself
      // could not be instantiated. When actually performing a substitution into
      // that pack expansion (e.g., when all template parameters have
      // corresponding
      // arguments), this type will be replaced with the
      // SubstTemplateTypeParmType at the current pack substitution index."
      UNSUPPORTED_CLANG_TYPE(SubstTemplateTypeParmPack);
    case TypeLoc::TemplateSpecialization: {
      // This refers to a particular class template, type alias template,
      // or template template parameter. Non-dependent template specializations
      // appear as different types.
      const auto &T = TypeLoc.castAs<TemplateSpecializationTypeLoc>();
      const SourceLocation &TNameLoc = T.getTemplateNameLoc();
      auto TemplateName = BuildNodeIdForTemplateName(
          T.getTypePtr()->getTemplateName(), TNameLoc);
      if (!TemplateName) {
        return TemplateName;
      }
      if (EmitRanges == IndexerASTVisitor::EmitRanges::Yes &&
          TNameLoc.isFileID()) {
        // Create a reference to the template instantiation that this type
        // refers to. If the type is dependent, create a reference to the
        // primary template.
        auto DeclNode = TemplateName.primary();
        auto *RD = T.getTypePtr()->getAsCXXRecordDecl();
        if (RD) {
          DeclNode = BuildNodeIdForDecl(RD);
        }
        if (auto RCC = ExplicitRangeInCurrentContext(
                RangeForSingleTokenFromSourceLocation(
                    *Observer.getSourceManager(), *Observer.getLangOptions(),
                    TNameLoc))) {
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
        ID = Observer.recordTappNode(TemplateName.primary(), TemplateArgsPtrs,
                                     TemplateArgsPtrs.size());
      }
    } break;
    case TypeLoc::Auto: {
      const auto *DT = dyn_cast<AutoType>(PT);
      QualType DeducedQT = DT->getDeducedType();
      if (DeducedQT.isNull()) {
        const auto &AutoT = TypeLoc.castAs<AutoTypeLoc>();
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
      const auto &T = TypeLoc.castAs<InjectedClassNameTypeLoc>();
      CXXRecordDecl *Decl = T.getDecl();
      if (!TypeAlreadyBuilt) {  // Leaf.
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
          auto Parent = GetDeclChildOf(Decl);
          auto Marks = MarkedSources.Generate(Decl);
          auto DeclNameId = BuildNameIdForDecl(Decl);
          ID = Observer.recordNominalTypeNode(
              DeclNameId,
              Marks.GenerateMarkedSource(
                  Observer.nodeIdForNominalTypeNode(DeclNameId)),
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
        const auto &T = TypeLoc.castAs<DependentNameTypeLoc>();
        const auto &NNS = T.getQualifierLoc();
        const auto &NameLoc = T.getNameLoc();
        ID = BuildNodeIdForDependentName(
            NNS, clang::DeclarationName(T.getTypePtr()->getIdentifier()),
            NameLoc, None(), EmitRanges);
      }
      break;
      UNSUPPORTED_CLANG_TYPE(DependentTemplateSpecialization);
    case TypeLoc::PackExpansion: {
      const auto &T = TypeLoc.castAs<PackExpansionTypeLoc>();
      const auto *DT = dyn_cast<PackExpansionType>(PT);
      auto PatternID(BuildNodeIdForType(
          T.getPatternLoc(),
          DT ? DT->getPattern().getTypePtr() : T.getPatternLoc().getTypePtr(),
          EmitRanges));
      if (!PatternID) {
        return PatternID;
      }
      if (TypeAlreadyBuilt) {
        break;
      }
      ID = PatternID;
    } break;
    case TypeLoc::ObjCObjectPointer: {
      // This is effectively a clone of PointerType since these two types
      // don't share ancestors and the code has some side effects.
      const auto &OPTL = TypeLoc.castAs<ObjCObjectPointerTypeLoc>();
      const auto *DT = dyn_cast<ObjCObjectPointerType>(PT);
      InEmitRanges = IndexerASTVisitor::EmitRanges::No;
      auto PointeeID(BuildNodeIdForType(OPTL.getPointeeLoc(),
                                        DT ? DT->getPointeeType().getTypePtr()
                                           : OPTL.getPointeeLoc().getTypePtr(),
                                        EmitRanges));
      if (!PointeeID) {
        return PointeeID;
      }
      if (SR.isValid() && SR.getBegin().isFileID()) {
        SR.setEnd(clang::Lexer::getLocForEndOfToken(
            OPTL.getStarLoc(), 0, /* offset from end of token */
            *Observer.getSourceManager(), *Observer.getLangOptions()));
      }
      if (TypeAlreadyBuilt) {
        break;
      }
      ID = ApplyBuiltinTypeConstructor("ptr", PointeeID);
    } break;
    case TypeLoc::ObjCInterface: {  // Leaf
      const auto &ITypeLoc = TypeLoc.castAs<ObjCInterfaceTypeLoc>();
      if (!TypeAlreadyBuilt) {
        const auto *IFace = ITypeLoc.getIFaceDecl();
        // Link to the implementation if we have one, otherwise link to the
        // interface. If we just have a forward declaration, link to the nominal
        // type node.
        if (const auto *Impl = IFace->getImplementation()) {
          Claimability = GraphObserver::Claimability::Unclaimable;
          ID = BuildNodeIdForDecl(Impl);
        } else if (!IsObjCForwardDecl(IFace)) {
          Claimability = GraphObserver::Claimability::Unclaimable;
          ID = BuildNodeIdForDecl(IFace);
        } else {
          // Thanks to the ODR, we shouldn't record multiple nominal type nodes
          // for the same TU: given distinct names, NameIds will be distinct,
          // there may be only one definition bound to each name, and we
          // memoize the NodeIds we give to types.
          auto Parent = GetDeclChildOf(IFace);
          auto Marks = MarkedSources.Generate(IFace);
          auto DeclNameId = BuildNameIdForDecl(IFace);
          ID = Observer.recordNominalTypeNode(
              DeclNameId,
              Marks.GenerateMarkedSource(
                  Observer.nodeIdForNominalTypeNode(DeclNameId)),
              Parent ? &Parent.primary() : nullptr);
        }
      }
    } break;
    case TypeLoc::ObjCObject: {
      const auto &ObjLoc = TypeLoc.castAs<ObjCObjectTypeLoc>();
      const auto *DT = dyn_cast<ObjCObjectType>(PT);
      MaybeFew<GraphObserver::NodeId> IFaceNode;
      if (const auto *IFace = DT->getInterface()) {
        IFaceNode = BuildNodeIdForType(ObjLoc.getBaseLoc(),
                                       IFace->getTypeForDecl(), EmitRanges);
      } else {
        IFaceNode = RecordIdTypeNode(DT, ObjLoc.getProtocolLocs());
      }
      if (!IFaceNode) {
        return IFaceNode;
      }

      if (ObjLoc.getNumTypeArgs() > 0) {
        std::vector<GraphObserver::NodeId> GenericArgIds;
        std::vector<const GraphObserver::NodeId *> GenericArgIdPtrs;
        GenericArgIds.reserve(ObjLoc.getNumTypeArgs());
        GenericArgIdPtrs.resize(ObjLoc.getNumTypeArgs(), nullptr);
        for (unsigned int i = 0; i < ObjLoc.getNumTypeArgs(); ++i) {
          const auto *TI = ObjLoc.getTypeArgTInfo(i);
          if (auto Arg = BuildNodeIdForType(TI->getTypeLoc(), TI->getType(),
                                            EmitRanges)) {
            GenericArgIds.push_back((Arg.primary()));
            GenericArgIdPtrs[i] = &GenericArgIds[i];
          } else {
            return Arg;
          }
        }
        if (!TypeAlreadyBuilt) {
          ID = Observer.recordTappNode(IFaceNode.primary(), GenericArgIdPtrs,
                                       GenericArgIdPtrs.size());
        }
      } else if (!TypeAlreadyBuilt) {
        ID = IFaceNode;
      }
    }; break;
    case TypeLoc::ObjCTypeParam: {
      const auto &OTPTL = TypeLoc.castAs<ObjCTypeParamTypeLoc>();
      const auto *OTPT = dyn_cast<ObjCTypeParamType>(PT);

      if (const auto *TPD = OTPT ? OTPT->getDecl() : OTPTL.getDecl()) {
        ID = BuildNodeIdForDecl(TPD);
      }

    } break;
    case TypeLoc::BlockPointer: {
      const auto &BPTL = TypeLoc.castAs<BlockPointerTypeLoc>();
      const auto &DT = dyn_cast<BlockPointerType>(PT);
      InEmitRanges = IndexerASTVisitor::EmitRanges::No;
      auto PointeeID(BuildNodeIdForType(BPTL.getPointeeLoc(),
                                        DT ? DT->getPointeeType().getTypePtr()
                                           : BPTL.getPointeeLoc().getTypePtr(),
                                        EmitRanges));
      if (!PointeeID) {
        return PointeeID;
      }
      if (SR.isValid() && SR.getBegin().isFileID()) {
        SR.setEnd(GetLocForEndOfToken(*Observer.getSourceManager(),
                                      *Observer.getLangOptions(),
                                      BPTL.getCaretLoc()));
      }
      if (TypeAlreadyBuilt) {
        break;
      }
      ID = ApplyBuiltinTypeConstructor("ptr", PointeeID);
    } break;
      UNSUPPORTED_CLANG_TYPE(Atomic);
      UNSUPPORTED_CLANG_TYPE(Pipe);
      // Attributed is handled in a special way at the top of this function so
      // it is impossible for the typeloc to be TypeLoc::Attributed.
      UNSUPPORTED_CLANG_TYPE(Attributed);
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
      SR = RangeForASTEntityFromSourceLocation(*Observer.getSourceManager(),
                                               *Observer.getLangOptions(),
                                               SR.getBegin());
    }
    if (auto RCC = ExplicitRangeInCurrentContext(SR)) {
      ID.Iter([&](const GraphObserver::NodeId &I) {
        Observer.recordTypeSpellingLocation(RCC.primary(), I, Claimability);
      });
    }
  }
  return ID;
}

MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::RecordIdTypeNode(
    const ObjCObjectType *T, ArrayRef<SourceLocation> ProtocolLocs) {
  CHECK(T->getInterface() == nullptr);

  const auto Protocols = T->getProtocols();
  // Use a multimap since it is sorted by key and we want our nodes sorted by
  // their (uncompressed) name. We want the items sorted by the original class
  // name because the user should be able to write down a union type
  // for the verifier and they can only do that if they know the order in which
  // the types will be passed as parameters.
  std::multimap<std::string, GraphObserver::NodeId> ProtocolNodes;
  unsigned i = 0;
  for (ObjCProtocolDecl *P : Protocols) {
    auto SL = ProtocolLocs[i++];
    auto PID = BuildNodeIdForDecl(P);
    auto SR = RangeForASTEntityFromSourceLocation(
        *Observer.getSourceManager(), *Observer.getLangOptions(), SL);
    if (auto ERCC = ExplicitRangeInCurrentContext(SR)) {
      Observer.recordDeclUseLocation(ERCC.primary(), PID);
    }
    ProtocolNodes.insert(std::pair<std::string, GraphObserver::NodeId>(
        P->getNameAsString(), PID));
  }
  if (ProtocolNodes.empty()) {
    return Observer.getNodeIdForBuiltinType("id");
  } else if (ProtocolNodes.size() == 1) {
    // We have something like id<P1>. This is a special case of the following
    // code that handles id<P1, P2> because *this* case can skip the
    // intermediate union tapp.
    return (ProtocolNodes.begin()->second);
  }
  // We have something like id<P1, P2>, which means this is a union of types
  // P1 and P2.

  std::vector<const GraphObserver::NodeId *> ProtocolNodePointers;
  ProtocolNodePointers.resize(ProtocolNodes.size(), nullptr);
  i = 0;
  for (const auto &PN : ProtocolNodes) {
    ProtocolNodePointers[i++] = &(PN.second);
  }
  // Create/find the Union node.
  auto UnionTApp = Observer.getNodeIdForBuiltinType("TypeUnion");
  return Observer.recordTappNode(UnionTApp, ProtocolNodePointers,
                                 ProtocolNodePointers.size());
}

// This is the synthesize statement.
//
// We don't do anything here because we don't want to draw edges to the
// synthesize statement. Any edges we draw to this statement are because of
// the synthesized IVarDecls in the AST which we visit separately.
//
// The following three scenarios produce synthesized ivar decls.
// * The synthesize statements declares a new ivar.
//   * @synthesize a = newvar;
//   * We get an ivar decl rooted at the new ivar name's source range.
// * It may specify no ivar, in which case the default value is used.
//   * @synthesize a;
//   * We get an ivar decl rooted at the property's name source range in the
//     synthesize statement.
// * This entire statement may be left out when modern versions of xcode are
//   used.
//   * We get an ivar decl rooted at the property's name source range in the
//     property declaration.
//
// In the future, we may want to draw some edge between the ivar and the
// property to show their relationship.
bool IndexerASTVisitor::VisitObjCPropertyImplDecl(
    const clang::ObjCPropertyImplDecl *Decl) {
  return true;
}

// This is like a typedef at a high level. It says that class A can be used
// instead of class B if class B does not exist.
bool IndexerASTVisitor::VisitObjCCompatibleAliasDecl(
    const clang::ObjCCompatibleAliasDecl *Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  // TODO(salguarnieri) Find a better way to parse the tokens to account for
  // macros with parameters.

  // Ugliness to get the ranges for the tokens in this decl since clang does
  // not give them to us. We expect something of the form:
  // @compatibility_alias AliasName OriginalClassName
  // Note that this does not work in the presence of macros with parameters.
  SourceRange AtSign = RangeForNameOfDeclaration(Decl);
  const auto KeywordRange(
      ConsumeToken(AtSign.getEnd(), clang::tok::identifier));
  const auto AliasRange(
      ConsumeToken(KeywordRange.getEnd(), clang::tok::identifier));
  const auto OrgClassRange(
      ConsumeToken(AliasRange.getEnd(), clang::tok::identifier));

  // Record a ref to the original type
  if (const auto &ERCC = ExplicitRangeInCurrentContext(OrgClassRange)) {
    const auto &ID = BuildNodeIdForDecl(Decl->getClassInterface());
    Observer.recordDeclUseLocation(ERCC.primary(), ID);
  }

  // Record the type alias
  const auto &OriginalInterface = Decl->getClassInterface();
  GraphObserver::NameId AliasID(BuildNameIdForDecl(Decl));
  auto AliasedTypeID(BuildNodeIdForDecl(OriginalInterface));
  auto AliasNode = Observer.recordTypeAliasNode(
      AliasID, AliasedTypeID, AliasedTypeID,
      Marks.GenerateMarkedSource(
          Observer.nodeIdForTypeAliasNode(AliasID, AliasedTypeID)));

  // Record the definition of this type alias
  MaybeRecordDefinitionRange(ExplicitRangeInCurrentContext(AliasRange),
                             AliasNode);
  AddChildOfEdgeToDeclContext(Decl, AliasNode);

  return true;
}

bool IndexerASTVisitor::VisitObjCImplementationDecl(
    const clang::ObjCImplementationDecl *ImplDecl) {
  auto Marks = MarkedSources.Generate(ImplDecl);
  SourceRange NameRange = RangeForNameOfDeclaration(ImplDecl);
  auto DeclNode = BuildNodeIdForDecl(ImplDecl);

  MaybeRecordDefinitionRange(
      RangeInCurrentContext(ImplDecl->isImplicit(), DeclNode, NameRange),
      DeclNode);
  AddChildOfEdgeToDeclContext(ImplDecl, DeclNode);

  FileID DeclFile =
      Observer.getSourceManager()->getFileID(ImplDecl->getLocation());
  if (auto Interface = ImplDecl->getClassInterface()) {
    if (auto NameRangeInContext = RangeInCurrentContext(ImplDecl->isImplicit(),
                                                        DeclNode, NameRange)) {
      // Draw the completion edge to this class's interface decl.
      if (!Interface->isImplicit()) {
        FileID InterfaceFile =
            Observer.getSourceManager()->getFileID(Interface->getLocation());
        GraphObserver::NodeId TargetDecl = BuildNodeIdForDecl(Interface);
        Observer.recordCompletionRange(
            NameRangeInContext.primary(), TargetDecl,
            InterfaceFile == DeclFile
                ? GraphObserver::Specificity::UniquelyCompletes
                : GraphObserver::Specificity::Completes);
      }
      RecordCompletesForRedecls(ImplDecl, NameRange, DeclNode);
    }
    ConnectToSuperClassAndProtocols(DeclNode, Interface);
  } else {
    LogErrorWithASTDump("Missing class interface", ImplDecl);
  }
  Observer.recordRecordNode(DeclNode, GraphObserver::RecordKind::Class,
                            GraphObserver::Completeness::Definition,
                            Marks.GenerateMarkedSource(DeclNode));
  return true;
}

// Categories and Classes are different things. You either have an
// ObjCImplementationDecl *OR* a ObjCCategoryImplDecl.
//
// Category implementations only occur in the case of true categories. They
// do not occur for an extension.
bool IndexerASTVisitor::VisitObjCCategoryImplDecl(
    const clang::ObjCCategoryImplDecl *ImplDecl) {
  auto Marks = MarkedSources.Generate(ImplDecl);
  SourceRange NameRange = RangeForASTEntityFromSourceLocation(
      *Observer.getSourceManager(), *Observer.getLangOptions(),
      ImplDecl->getCategoryNameLoc());
  auto ImplDeclNode = BuildNodeIdForDecl(ImplDecl);
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(ImplDecl->isImplicit(), ImplDeclNode, NameRange),
      ImplDeclNode);
  AddChildOfEdgeToDeclContext(ImplDecl, ImplDeclNode);

  FileID ImplDeclFile =
      Observer.getSourceManager()->getFileID(ImplDecl->getCategoryNameLoc());

  Observer.recordRecordNode(ImplDeclNode, GraphObserver::RecordKind::Category,
                            GraphObserver::Completeness::Definition,
                            Marks.GenerateMarkedSource(ImplDeclNode));

  if (auto CategoryDecl = ImplDecl->getCategoryDecl()) {
    if (auto NameRangeInContext = ExplicitRangeInCurrentContext(NameRange)) {
      // Draw the completion edge to this category's decl.
      if (!CategoryDecl->isImplicit()) {
        FileID DeclFile =
            Observer.getSourceManager()->getFileID(CategoryDecl->getLocation());
        GraphObserver::NodeId DeclNode = BuildNodeIdForDecl(CategoryDecl);
        Observer.recordCompletionRange(
            NameRangeInContext.primary(), DeclNode,
            DeclFile == ImplDeclFile
                ? GraphObserver::Specificity::UniquelyCompletes
                : GraphObserver::Specificity::Completes);
      }
      RecordCompletesForRedecls(ImplDecl, NameRange, ImplDeclNode);
    }
    // Record a use of the category decl where the name is specified.

    // It should be impossible to be a class extension and have an
    // ObjCCategoryImplDecl.
    if (CategoryDecl->IsClassExtension()) {
      LOG(ERROR) << "Class extensions should not have a category impl.";
      return true;
    }
    auto Range = RangeForASTEntityFromSourceLocation(
        *Observer.getSourceManager(), *Observer.getLangOptions(),
        ImplDecl->getCategoryNameLoc());
    if (auto RCC = ExplicitRangeInCurrentContext(Range)) {
      auto ID = BuildNodeIdForDecl(CategoryDecl);
      Observer.recordDeclUseLocation(RCC.primary(), ID,
                                     GraphObserver::Claimability::Unclaimable);
    }
  } else {
    LogErrorWithASTDump("Missing category decl", ImplDecl);
  }

  if (auto BaseClassInterface = ImplDecl->getClassInterface()) {
    ConnectCategoryToBaseClass(ImplDeclNode, BaseClassInterface);

    // Link the class interface name to the class interface
    auto ClassInterfaceNode = BuildNodeIdForDecl(BaseClassInterface);
    // The location for the category decl is actually the location of the
    // interface name.
    const SourceRange &IFaceNameRange = RangeForASTEntityFromSourceLocation(
        *Observer.getSourceManager(), *Observer.getLangOptions(),
        ImplDecl->getLocation());
    if (auto RCC = ExplicitRangeInCurrentContext(IFaceNameRange)) {
      Observer.recordDeclUseLocation(RCC.primary(), ClassInterfaceNode);
    }
  } else {
    LogErrorWithASTDump("Missing category impl class interface", ImplDecl);
  }

  return true;
}

void IndexerASTVisitor::ConnectToSuperClassAndProtocols(
    const GraphObserver::NodeId BodyDeclNode,
    const clang::ObjCInterfaceDecl *IFace) {
  if (IsObjCForwardDecl(IFace)) {
    // This is a forward declaration, so it won't have superclass or protocol
    // info
    return;
  }

  if (const auto *SCTSI = IFace->getSuperClassTInfo()) {
    // If EmitRanges is Yes, we will emit a ref from the use of the superclass
    // in this interface declaration (IFace) to the superclass implementation.
    //
    // If EmitRanges is No, we will *not* emit a ref from the user of the
    // superclass to the implementation of the superclass.
    //
    // We choose to set EmitRanges to Yes because it matches the behavior of
    // other parts of kythe that link to the Impl if possible. In a following
    // code block, we also emit a ref from the use of the superclass to the
    // declaration of the superclass. It should be up to the UI to follow the
    // link that makes the most sense in a given context (probably relying on
    // the user to help clarify what they want).
    if (auto SCType =
            BuildNodeIdForType(SCTSI->getTypeLoc(), EmitRanges::Yes)) {
      Observer.recordExtendsEdge(BodyDeclNode, SCType.primary(),
                                 false /* isVirtual */,
                                 clang::AccessSpecifier::AS_none);
    }
  }
  // Draw a ref edge from the superclass usage in the interface declaration to
  // the superclass declaration.
  if (auto SC = IFace->getSuperClass()) {
    auto SuperRange = RangeForASTEntityFromSourceLocation(
        *Observer.getSourceManager(), *Observer.getLangOptions(),
        IFace->getSuperClassLoc());
    if (auto SCRCC = ExplicitRangeInCurrentContext(SuperRange)) {
      auto SCID = BuildNodeIdForDecl(SC);
      Observer.recordDeclUseLocation(SCRCC.primary(), SCID,
                                     GraphObserver::Claimability::Unclaimable);
    }
  }

  ConnectToProtocols(BodyDeclNode, IFace->protocol_loc_begin(),
                     IFace->protocol_loc_end(), IFace->protocol_begin(),
                     IFace->protocol_end());
}

void IndexerASTVisitor::ConnectToProtocols(
    const GraphObserver::NodeId BodyDeclNode,
    clang::ObjCProtocolList::loc_iterator locStart,
    clang::ObjCProtocolList::loc_iterator locEnd,
    clang::ObjCProtocolList::iterator itStart,
    clang::ObjCProtocolList::iterator itEnd) {
  // The location of the protocols in the interface decl and the protocol
  // decls are stored in two parallel arrays, iterate through them at the same
  // time.
  auto PLocIt = locStart;
  auto PIt = itStart;
  // The termination condition should only need to check one of the iterators
  // since they should have the exact same number of elements but checking
  // them both keeps us safe.
  for (; PLocIt != locEnd && PIt != itEnd; ++PLocIt, ++PIt) {
    Observer.recordExtendsEdge(BodyDeclNode, BuildNodeIdForDecl(*PIt),
                               false /* isVirtual */,
                               clang::AccessSpecifier::AS_none);
    auto Range = RangeForASTEntityFromSourceLocation(
        *Observer.getSourceManager(), *Observer.getLangOptions(), *PLocIt);
    if (auto ERCC = ExplicitRangeInCurrentContext(Range)) {
      auto PID = BuildNodeIdForDecl(*PIt);
      Observer.recordDeclUseLocation(ERCC.primary(), PID,
                                     GraphObserver::Claimability::Unclaimable);
    }
  }
}

// Outline for the spirit of how generic types are handled in the kythe graph
// (the following may be implemented out of order and may be partially
// implemented in other methods):
//
// Create a node for the generic class (ObjCInterfaceDecl)
// If there are type arguments:
//   Create an abs node
//   Create an absvar nodes for the type arguments
//   Make the class a childof the abs node
//   Make the type arguments parameters of the abs node
// If the class is generic, there is no source range that corresponds to the
// record node created for the class.
bool IndexerASTVisitor::VisitObjCInterfaceDecl(
    const clang::ObjCInterfaceDecl *Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  GraphObserver::NodeId BodyDeclNode(Observer.getDefaultClaimToken(), "");
  GraphObserver::NodeId DeclNode(Observer.getDefaultClaimToken(), "");

  // If we have type arguments, treat this as a generic type and indirect
  // through an abs node.
  if (const auto *TPL = Decl->getTypeParamList()) {
    BodyDeclNode = BuildNodeIdForDecl(Decl, 0);
    DeclNode = RecordGenericClass(Decl, TPL, BodyDeclNode);
  } else {
    BodyDeclNode = BuildNodeIdForDecl(Decl);
    DeclNode = BodyDeclNode;
  }

  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode);

  AddChildOfEdgeToDeclContext(Decl, DeclNode);

  auto Completeness = IsObjCForwardDecl(Decl)
                          ? GraphObserver::Completeness::Incomplete
                          : GraphObserver::Completeness::Complete;
  Observer.recordRecordNode(BodyDeclNode, GraphObserver::RecordKind::Class,
                            Completeness, None());
  Observer.recordMarkedSource(DeclNode, Marks.GenerateMarkedSource(DeclNode));
  RecordCompletesForRedecls(Decl, NameRange, BodyDeclNode);
  ConnectToSuperClassAndProtocols(BodyDeclNode, Decl);
  return true;
}

// Categories and Classes are different things. You either have an
// ObjCInterfaceDecl *OR* a ObjCCategoryDecl.
bool IndexerASTVisitor::VisitObjCCategoryDecl(
    const clang::ObjCCategoryDecl *Decl) {
  // Use the category name as our name range. If this is an extension and has
  // no category name, use the name range for the interface decl.
  auto Marks = MarkedSources.Generate(Decl);
  SourceRange NameRange;
  if (Decl->IsClassExtension()) {
    NameRange = RangeForNameOfDeclaration(Decl);
  } else {
    NameRange = RangeForASTEntityFromSourceLocation(
        *Observer.getSourceManager(), *Observer.getLangOptions(),
        Decl->getCategoryNameLoc());
  }

  auto DeclNode = BuildNodeIdForDecl(Decl);
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode);
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  Observer.recordRecordNode(DeclNode, GraphObserver::RecordKind::Category,
                            GraphObserver::Completeness::Complete,
                            Marks.GenerateMarkedSource(DeclNode));
  RecordCompletesForRedecls(Decl, NameRange, DeclNode);
  if (auto BaseClassInterface = Decl->getClassInterface()) {
    ConnectCategoryToBaseClass(DeclNode, BaseClassInterface);

    // Link the class interface name to the class interface
    auto ClassInterfaceNode = BuildNodeIdForDecl(BaseClassInterface);
    // The location for the category decl is actually the location of the
    // interface name.
    const SourceRange &IFaceNameRange = RangeForASTEntityFromSourceLocation(
        *Observer.getSourceManager(), *Observer.getLangOptions(),
        Decl->getLocation());
    if (auto RCC = ExplicitRangeInCurrentContext(IFaceNameRange)) {
      Observer.recordDeclUseLocation(RCC.primary(), ClassInterfaceNode);
    }
  } else {
    LogErrorWithASTDump("Missing category decl class interface", Decl);
  }

  return true;
}

// This method is not inlined because it is important that we do the same thing
// for category declarations and category implementations.
void IndexerASTVisitor::ConnectCategoryToBaseClass(
    const GraphObserver::NodeId &DeclNode, const ObjCInterfaceDecl *IFace) {
  auto ClassTypeId = BuildNodeIdForDecl(IFace);
  Observer.recordCategoryExtendsEdge(DeclNode, ClassTypeId);
}

void IndexerASTVisitor::RecordCompletesForRedecls(
    const Decl *Decl, const SourceRange &NameRange,
    const GraphObserver::NodeId &DeclNode) {
  // Don't draw completion edges if this is a forward declared class in
  // Objective-C because forward declarations don't complete anything.
  if (const auto *I = dyn_cast<clang::ObjCInterfaceDecl>(Decl)) {
    if (IsObjCForwardDecl(I)) {
      return;
    }
  }

  FileID DeclFile = Observer.getSourceManager()->getFileID(Decl->getLocation());
  if (auto NameRangeInContext =
          RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange)) {
    for (const auto *NextDecl : Decl->redecls()) {
      // It's not useful to draw completion edges to implicit forward
      // declarations, nor is it useful to declare that a definition completes
      // itself.
      if (NextDecl != Decl && !NextDecl->isImplicit()) {
        FileID NextDeclFile =
            Observer.getSourceManager()->getFileID(NextDecl->getLocation());
        GraphObserver::NodeId TargetDecl = BuildNodeIdForDecl(NextDecl);
        Observer.recordCompletionRange(
            NameRangeInContext.primary(), TargetDecl,
            NextDeclFile == DeclFile
                ? GraphObserver::Specificity::UniquelyCompletes
                : GraphObserver::Specificity::Completes);
      }
    }
  }
}

bool IndexerASTVisitor::VisitObjCProtocolDecl(
    const clang::ObjCProtocolDecl *Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  auto DeclNode = BuildNodeIdForDecl(Decl);

  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode);
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  Observer.recordInterfaceNode(DeclNode, Marks.GenerateMarkedSource(DeclNode));
  ConnectToProtocols(DeclNode, Decl->protocol_loc_begin(),
                     Decl->protocol_loc_end(), Decl->protocol_begin(),
                     Decl->protocol_end());
  return true;
}

bool IndexerASTVisitor::VisitObjCMethodDecl(const clang::ObjCMethodDecl *Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  auto Node = BuildNodeIdForDecl(Decl);
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  // TODO(salguarnieri) Use something more honest for the name in the marked
  // source. The "name" of an Objective-C method is not a single range in the
  // source code. Ex: -(void) myFunc:(int)data except:(int)moreData should have
  // the name "myFunc:except:".
  Marks.set_name_range(NameRange);
  Marks.set_marked_source_end(Decl->getSourceRange().getEnd());
  auto NameRangeInContext(
      RangeInCurrentContext(Decl->isImplicit(), Node, NameRange));
  MaybeRecordDefinitionRange(NameRangeInContext, Node);
  bool IsFunctionDefinition = Decl->isThisDeclarationADefinition();
  unsigned ParamNumber = 0;
  for (const auto *Param : Decl->parameters()) {
    ConnectParam(Decl, Node, IsFunctionDefinition, ParamNumber++, Param, false);
  }

  MaybeFew<GraphObserver::NodeId> FunctionType =
      CreateObjCMethodTypeNode(Decl, EmitRanges::Yes);

  if (FunctionType) {
    Observer.recordTypeEdge(Node, FunctionType.primary());
  }

  // Record overrides edges
  SmallVector<const ObjCMethodDecl *, 4> overrides;
  Decl->getOverriddenMethods(overrides);
  for (const auto &O : overrides) {
    Observer.recordOverridesEdge(Node, BuildNodeIdForDecl(O));
  }
  if (!overrides.empty()) {
    MapOverrideRoots(Decl, [&](const ObjCMethodDecl *R) {
      Observer.recordOverridesRootEdge(Node, BuildNodeIdForDecl(R));
    });
  }

  AddChildOfEdgeToDeclContext(Decl, Node);
  GraphObserver::FunctionSubkind Subkind = GraphObserver::FunctionSubkind::None;
  if (!IsFunctionDefinition) {
    Observer.recordFunctionNode(Node, GraphObserver::Completeness::Incomplete,
                                Subkind, Marks.GenerateMarkedSource(Node));

    // If this is a decl in an extension, we need to connect it to its
    // implementation here.
    //
    // When we are visiting the definition for a method decl from an extension,
    // we don't have a way to get back to the declaration. The true method
    // decl does not appear in the list of redecls and it is not returned by
    // getCanonicalDecl. Furthermore, there is no way to get access to the
    // extension declaration. Since class implementations do not mention
    // extensions, there is nothing for us to use.
    //
    // There may be unexpected behavior in getCanonicalDecl because it only
    // covers the following cases:
    //  * When the declaration is a child of ObjCInterfaceDecl and the
    //    definition is a child of ObjCImplementationDecl.
    //  * When the declaration is a child of ObjCCategoryDecl and the
    //    definition is a child of ObjCCategeoryImplDecl.
    // It does not cover the following case:
    //  * When the declaration is a child of ObjCCategoryDecl and the
    //    definition is a child of ObjCImplementationDecl. This occurs in an
    //    extension.
    if (auto CategoryDecl =
            dyn_cast<ObjCCategoryDecl>(Decl->getDeclContext())) {
      if (CategoryDecl->IsClassExtension() &&
          CategoryDecl->getClassInterface() != nullptr &&
          CategoryDecl->getClassInterface()->getImplementation() != nullptr) {
        ObjCImplementationDecl *ClassImpl =
            CategoryDecl->getClassInterface()->getImplementation();
        if (auto MethodImpl = ClassImpl->getMethod(Decl->getSelector(),
                                                   Decl->isInstanceMethod())) {
          if (MethodImpl != Decl) {
            FileID DeclFile =
                Observer.getSourceManager()->getFileID(Decl->getLocation());
            FileID ImplFile = Observer.getSourceManager()->getFileID(
                MethodImpl->getLocation());
            auto ImplNode = BuildNodeIdForDecl(MethodImpl);
            SourceRange ImplNameRange = RangeForNameOfDeclaration(MethodImpl);
            auto ImplNameRangeInContext(RangeInCurrentContext(
                MethodImpl->isImplicit(), ImplNode, ImplNameRange));
            if (ImplNameRangeInContext) {
              Observer.recordCompletionRange(
                  ImplNameRangeInContext.primary(), BuildNodeIdForDecl(Decl),
                  DeclFile == ImplFile
                      ? GraphObserver::Specificity::UniquelyCompletes
                      : GraphObserver::Specificity::Completes);
            }
          }
        }
      }
    }
    return true;
  }

  if (NameRangeInContext) {
    FileID DefnFile =
        Observer.getSourceManager()->getFileID(Decl->getLocation());
    const GraphObserver::Range &DeclNameRangeInContext =
        NameRangeInContext.primary();

    // If we want to draw an edge from method impl to property we can modify the
    // following code:
    // auto *propertyDecl = Decl->findPropertyDecl(true /* checkOverrides */);
    // if (propertyDecl != nullptr)  ...

    // This is necessary if this definition comes from an implicit method from
    // a property.
    auto CD = Decl->getCanonicalDecl();

    // Do not draw self-completion edges.
    if (Decl != CD) {
      FileID CDDeclFile =
          Observer.getSourceManager()->getFileID(CD->getLocation());
      Observer.recordCompletionRange(
          DeclNameRangeInContext, BuildNodeIdForDecl(CD),
          CDDeclFile == DefnFile ? GraphObserver::Specificity::UniquelyCompletes
                                 : GraphObserver::Specificity::Completes);
    }

    // Connect all other redecls to this definition with a completion edge.
    for (const auto *NextDecl : Decl->redecls()) {
      // Do not draw self-completion edges and do not draw an edge for the
      // canonical decl again.
      if (NextDecl != Decl && NextDecl != CD) {
        FileID RedeclFile =
            Observer.getSourceManager()->getFileID(NextDecl->getLocation());
        Observer.recordCompletionRange(
            DeclNameRangeInContext, BuildNodeIdForDecl(NextDecl),
            RedeclFile == DefnFile
                ? GraphObserver::Specificity::UniquelyCompletes
                : GraphObserver::Specificity::Completes);
      }
    }
  }
  Observer.recordFunctionNode(Node, GraphObserver::Completeness::Definition,
                              Subkind, Marks.GenerateMarkedSource(Node));
  return true;
}

// TODO(salguarnieri) Do we need to record a use for the parameter type?
void IndexerASTVisitor::ConnectParam(const Decl *Decl,
                                     const GraphObserver::NodeId &FuncNode,
                                     bool IsFunctionDefinition,
                                     const unsigned int ParamNumber,
                                     const ParmVarDecl *Param,
                                     bool DeclIsImplicit) {
  auto Marks = MarkedSources.Generate(Param);
  GraphObserver::NodeId VarNodeId(BuildNodeIdForDecl(Param));
  SourceRange Range = RangeForNameOfDeclaration(Param);
  Marks.set_name_range(Range);
  Marks.set_implicit(Job->UnderneathImplicitTemplateInstantiation ||
                     DeclIsImplicit);
  Marks.set_marked_source_end(GetLocForEndOfToken(
      *Observer.getSourceManager(), *Observer.getLangOptions(),
      Param->getSourceRange().getEnd()));
  Observer.recordVariableNode(VarNodeId,
                              IsFunctionDefinition
                                  ? GraphObserver::Completeness::Definition
                                  : GraphObserver::Completeness::Incomplete,
                              GraphObserver::VariableSubkind::None,
                              Marks.GenerateMarkedSource(VarNodeId));
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Param->isImplicit() || Decl->isImplicit(),
                            VarNodeId, Range),
      VarNodeId);
  Observer.recordParamEdge(FuncNode, ParamNumber, VarNodeId);
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

// TODO(salguarnieri) Think about merging with VisitFieldDecl
bool IndexerASTVisitor::VisitObjCPropertyDecl(
    const clang::ObjCPropertyDecl *Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode);
  // TODO(zarko): Record completeness data. This is relevant for static fields,
  // which may be declared along with a complete class definition but later
  // defined in a separate translation unit.
  Observer.recordVariableNode(
      DeclNode, GraphObserver::Completeness::Definition,
      // TODO(salguarnieri) Think about making a new subkind for properties.
      GraphObserver::VariableSubkind::Field,
      Marks.GenerateMarkedSource(DeclNode));
  if (const auto *TSI = Decl->getTypeSourceInfo()) {
    // TODO(zarko): Record storage classes for fields.
    AscribeSpelledType(TSI->getTypeLoc(), Decl->getType(), DeclNode);
  } else if (auto TyNodeId = BuildNodeIdForType(Decl->getType())) {
    Observer.recordTypeEdge(DeclNode, TyNodeId.primary());
  }
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  return true;
}

bool IndexerASTVisitor::VisitObjCIvarRefExpr(
    const clang::ObjCIvarRefExpr *IRE) {
  return VisitDeclRefOrIvarRefExpr(IRE, IRE->getDecl(), IRE->getLocation());
}

bool IndexerASTVisitor::VisitObjCMessageExpr(
    const clang::ObjCMessageExpr *Expr) {
  if (Job->BlameStack.empty()) {
    return true;
  }

  if (Expr->isClassMessage()) {
    const QualType &CR = Expr->getClassReceiver();
    if (CR.isNull()) {
      // TODO(salguarnieri) Figure out why this happens and if it is normal. The
      // comments at
      // https://clang.llvm.org/doxygen/classclang_1_1ObjCMessageExpr.html#a6457400eb37b74bf08efc178d6474e07
      // claim that getClassReceiver should only return null if this is not
      // a class message.
      LogErrorWithASTDump("Class receiver was null", Expr);
    } else if (auto ID = BuildNodeIdForType(CR)) {
      const SourceLocation &Loc = Expr->getReceiverRange().getBegin();
      if (Loc.isValid() && Loc.isFileID()) {
        const SourceRange SR(RangeForSingleTokenFromSourceLocation(
            *Observer.getSourceManager(), *Observer.getLangOptions(), Loc));
        if (auto ERCC = ExplicitRangeInCurrentContext(SR)) {
          Observer.recordTypeSpellingLocation(
              ERCC.primary(), ID.primary(),
              GraphObserver::Claimability::Claimable);
        }
      }
    }
  }

  // The end of the source range for ObjCMessageExpr is the location of the
  // right brace. We actually want to include the right brace in the range
  // we record, so get the location *after* the right brace.
  const auto AfterBrace =
      ConsumeToken(Expr->getLocEnd(), clang::tok::r_square).getEnd();
  const SourceRange SR(Expr->getLocStart(), AfterBrace);
  if (auto RCC = ExplicitRangeInCurrentContext(SR)) {
    // This does not take dynamic dispatch into account when looking for the
    // method definition.
    if (const auto *Callee = FindMethodDefn(Expr->getMethodDecl(),
                                            Expr->getReceiverInterface())) {
      const GraphObserver::NodeId &DeclId = BuildNodeIdForRefToDecl(Callee);
      RecordCallEdges(RCC.primary(), DeclId);

      // We only record a ref if we have successfully recorded a ref/call.
      // If we don't have any selectors, just use the same span as the ref/call.
      if (Expr->getNumSelectorLocs() == 0) {
        Observer.recordDeclUseLocation(
            RCC.primary(), DeclId, GraphObserver::Claimability::Unclaimable);
      } else {
        // TODO Record multiple ranges, one for each selector.
        // For now, just record the range for the first selector. This should
        // make it easier for frontends to make use of this data.
        const SourceLocation &Loc = Expr->getSelectorLoc(0);
        if (Loc.isValid() && Loc.isFileID()) {
          const SourceRange &range = RangeForSingleTokenFromSourceLocation(
              *Observer.getSourceManager(), *Observer.getLangOptions(), Loc);
          if (auto R = ExplicitRangeInCurrentContext(range)) {
            Observer.recordDeclUseLocation(
                R.primary(), DeclId, GraphObserver::Claimability::Unclaimable);
          }
        }
      }
    }
  }
  return true;
}

const ObjCMethodDecl *IndexerASTVisitor::FindMethodDefn(
    const ObjCMethodDecl *MD, const ObjCInterfaceDecl *I) {
  if (MD == nullptr || I == nullptr || MD->isThisDeclarationADefinition()) {
    return MD;
  }
  // If we can, look in the implementation, otherwise we look in the interface.
  const ObjCContainerDecl *CD = I->getImplementation();
  if (CD == nullptr) {
    CD = I;
  }
  if (const auto *MI =
          CD->getMethod(MD->getSelector(), MD->isInstanceMethod())) {
    return MI;
  }
  return MD;
}

bool IndexerASTVisitor::VisitObjCPropertyRefExpr(
    const clang::ObjCPropertyRefExpr *Expr) {
  // Both implicit and explicit properties will provide a backing method decl.
  ObjCMethodDecl *MD = nullptr;
  ObjCPropertyDecl *PD = nullptr;
  if (Expr->isImplicitProperty()) {
    // TODO(salguarnieri) Create test cases for implicit properties.
    MD = Expr->isMessagingGetter() ? Expr->getImplicitPropertyGetter()
                                   : Expr->getImplicitPropertySetter();
  } else if ((PD = Expr->getExplicitProperty())) {
    MD = Expr->isMessagingGetter() ? PD->getGetterMethodDecl()
                                   : PD->getSetterMethodDecl();
  } else {
    // If this is an explicit property but we cannot get the property, we
    // need to stop.
    return true;
  }

  // Record the a "field" access *and* a method call.
  auto SL = Expr->getLocation();
  if (SL.isValid()) {
    // This gives us the property name. If we just call Expr->getSourceRange()
    // we just get the range for the object's name.
    SourceRange SR = RangeForASTEntityFromSourceLocation(
        *Observer.getSourceManager(), *Observer.getLangOptions(), SL);
    auto StmtId = BuildNodeIdForImplicitStmt(Expr);
    if (auto RCC = RangeInCurrentContext(StmtId, SR)) {
      // Record the "field" access if this has an explicit property.
      if (PD != nullptr) {
        GraphObserver::NodeId DeclId = BuildNodeIdForDecl(PD);
        Observer.recordDeclUseLocation(
            RCC.primary(), DeclId, GraphObserver::Claimability::Unclaimable);
        for (const auto &S : Supports) {
          S->InspectDeclRef(*this, SL, RCC.primary(), DeclId, PD);
        }
      }

      // Record the method call.

      // TODO(salguarnieri) Try to prevent the call edge unless the user has
      // defined a custom setter or getter. This is a non-trivial
      // because the user may provide a custom implementation using the
      // default name. Otherwise, we could just test the method decl
      // location.
      if (MD != nullptr) {
        RecordCallEdges(RCC.primary(), BuildNodeIdForRefToDecl(MD));
      }
    }
  }

  return true;
}

MaybeFew<GraphObserver::NodeId> IndexerASTVisitor::CreateObjCMethodTypeNode(
    const clang::ObjCMethodDecl *MD, EmitRanges EmitRanges) {
  std::vector<GraphObserver::NodeId> NodeIds;
  std::vector<const GraphObserver::NodeId *> NodeIdPtrs;
  // If we are in an implicit method (for example: property access), we may
  // not get return type source information and we will have to rely on the
  // QualType provided by getReturnType.
  auto R = MD->getReturnTypeSourceInfo();
  auto ReturnType =
      R ? BuildNodeIdForType(R->getTypeLoc(), R->getType().getTypePtr(),
                             EmitRanges)
        : BuildNodeIdForType(MD->getReturnType());
  if (!ReturnType) {
    return ReturnType;
  }

  NodeIds.push_back(ReturnType.primary());
  for (auto PVD : MD->parameters()) {
    if (PVD) {
      auto TSI = PVD->getTypeSourceInfo();
      auto ParamType =
          TSI ? BuildNodeIdForType(
                    PVD->getTypeSourceInfo()->getTypeLoc(),
                    PVD->getTypeSourceInfo()->getTypeLoc().getTypePtr(),
                    EmitRanges)
              : BuildNodeIdForType(PVD->getType());
      if (!ParamType) {
        return ParamType;
      }
      NodeIds.push_back(ParamType.primary());
    }
  }

  for (size_t I = 0; I < NodeIds.size(); ++I) {
    NodeIdPtrs.push_back(&NodeIds[I]);
  }
  // TODO(salguarnieri) Make this a constant somewhere
  const char *Tycon = "fn";
  return Observer.recordTappNode(Observer.getNodeIdForBuiltinType(Tycon),
                                 NodeIdPtrs, NodeIdPtrs.size());
}

void IndexerASTVisitor::LogErrorWithASTDump(const std::string &msg,
                                            const clang::Decl *Decl) const {
  std::string s;
  llvm::raw_string_ostream ss(s);
  Decl->dump(ss);
  LOG(ERROR) << msg << " :" << std::endl << s;
}

void IndexerASTVisitor::LogErrorWithASTDump(const std::string &msg,
                                            const clang::Expr *Expr) const {
  std::string s;
  llvm::raw_string_ostream ss(s);
  Expr->dump(ss);
  LOG(ERROR) << msg << " :" << std::endl << s;
}

}  // namespace kythe
