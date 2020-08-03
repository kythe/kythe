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

#include "IndexerASTHooks.h"

#include <algorithm>
#include <tuple>

#include "GraphObserver.h"
#include "absl/flags/flag.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
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
#include "clang/Basic/Builtins.h"
#include "clang/Index/USRGeneration.h"
#include "clang/Lex/Lexer.h"
#include "clang/Sema/Lookup.h"
#include "indexed_parent_iterator.h"
#include "kythe/cxx/common/scope_guard.h"
#include "kythe/cxx/indexer/cxx/clang_range_finder.h"
#include "kythe/cxx/indexer/cxx/clang_utils.h"
#include "kythe/cxx/indexer/cxx/marked_source.h"
#include "kythe/cxx/indexer/cxx/node_set.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

ABSL_FLAG(bool, experimental_alias_template_instantiations, false,
          "Ignore template instantation information when generating IDs.");
ABSL_FLAG(bool, experimental_threaded_claiming, false,
          "Defer answering claims and submit them in bulk when possible.");
ABSL_FLAG(bool, emit_anchors_on_builtins, true,
          "Emit anchors on builtin types like int and float.");

namespace kythe {

using namespace clang;

using Claimability = GraphObserver::Claimability;
using NodeId = GraphObserver::NodeId;

void* NullGraphObserver::NullClaimToken::NullClaimTokenClass = nullptr;

namespace {
/// Decides whether `Tok` can be used to quote an identifier.
bool TokenQuotesIdentifier(const clang::SourceManager& SM,
                           const clang::Token& Tok) {
  switch (Tok.getKind()) {
    case tok::TokenKind::pipe:
      return true;
    case tok::TokenKind::unknown: {
      bool Invalid = false;
      if (const char* TokChar =
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

absl::optional<llvm::StringRef> GetDeclarationName(
    const clang::DeclarationName& Name, bool IgnoreUnimplemented) {
  switch (Name.getNameKind()) {
    case clang::DeclarationName::Identifier:
      return Name.getAsIdentifierInfo()->getName();
    case clang::DeclarationName::CXXConstructorName:
      return "#ctor";
    case clang::DeclarationName::CXXDestructorName:
      return "#dtor";
// TODO(zarko): Fill in the remaining relevant DeclarationName cases.
#define UNEXPECTED_DECLARATION_NAME_KIND(kind)                         \
  case clang::DeclarationName::kind:                                   \
    CHECK(IgnoreUnimplemented) << "Unexpected DeclaraionName::" #kind; \
    return absl::nullopt;
      UNEXPECTED_DECLARATION_NAME_KIND(ObjCZeroArgSelector);
      UNEXPECTED_DECLARATION_NAME_KIND(ObjCOneArgSelector);
      UNEXPECTED_DECLARATION_NAME_KIND(ObjCMultiArgSelector);
      UNEXPECTED_DECLARATION_NAME_KIND(CXXConversionFunctionName);
      UNEXPECTED_DECLARATION_NAME_KIND(CXXOperatorName);
      UNEXPECTED_DECLARATION_NAME_KIND(CXXLiteralOperatorName);
      UNEXPECTED_DECLARATION_NAME_KIND(CXXUsingDirective);
      UNEXPECTED_DECLARATION_NAME_KIND(CXXDeductionGuideName);
#undef UNEXPECTED_DECLARATION_NAME_KIND
  }
  return absl::nullopt;
}

/// \brief Finds the first CXXConstructExpr child of the given
/// CXXFunctionalCastExpr.
const clang::CXXConstructExpr* FindConstructExpr(
    const clang::CXXFunctionalCastExpr* E) {
  for (const auto* Child : E->children()) {
    if (isa<clang::CXXConstructExpr>(Child)) {
      return dyn_cast<clang::CXXConstructExpr>(Child);
    }
  }
  return nullptr;
}

template <typename F>
void MapOverrideRoots(const clang::CXXMethodDecl* M, const F& Fn) {
  if (M->size_overridden_methods() == 0) {
    Fn(M);
  } else {
    for (const auto& PM : M->overridden_methods()) {
      MapOverrideRoots(PM, Fn);
    }
  }
}

template <typename F>
void MapOverrideRoots(const clang::ObjCMethodDecl* M, const F& Fn) {
  if (!M->isOverriding()) {
    Fn(M);
  } else {
    SmallVector<const ObjCMethodDecl*, 4> overrides;
    M->getOverriddenMethods(overrides);
    for (const auto& PM : overrides) {
      MapOverrideRoots(PM, Fn);
    }
  }
}

clang::QualType FollowAliasChain(const clang::TypedefNameDecl* TND) {
  clang::Qualifiers Qs;
  clang::QualType QT;
  for (;;) {
    // We'll assume that the alias chain stops as soon as we hit a non-alias.
    // This does not attempt to dereference aliases in template parameters
    // (or even aliases underneath pointers, etc).
    QT = TND->getUnderlyingType();
    Qs.addQualifiers(QT.getQualifiers());
    if (auto* TTD = dyn_cast<TypedefType>(QT.getTypePtr())) {
      TND = TTD->getDecl();
    } else if (auto* ET = dyn_cast<ElaboratedType>(QT.getTypePtr())) {
      if (auto* TDT = dyn_cast<TypedefType>(ET->getNamedType().getTypePtr())) {
        TND = TDT->getDecl();
        Qs.addQualifiers(ET->getNamedType().getQualifiers());
      } else {
        return QualType(QT.getTypePtr(), Qs.getFastQualifiers());
      }
    } else {
      // We lose fidelity with getFastQualifiers.
      return QualType(QT.getTypePtr(), Qs.getFastQualifiers());
    }
  }
}

/// \brief Attempt to find the template parameters bound immediately by `DC`.
/// \return null if no parameters could be found.
clang::TemplateParameterList* GetTypeParameters(const clang::DeclContext* DC) {
  if (!DC) {
    return nullptr;
  }
  if (const auto* TemplateContext = dyn_cast<clang::TemplateDecl>(DC)) {
    return TemplateContext->getTemplateParameters();
  } else if (const auto* CTPSD =
                 dyn_cast<ClassTemplatePartialSpecializationDecl>(DC)) {
    return CTPSD->getTemplateParameters();
  } else if (const auto* RD = dyn_cast<CXXRecordDecl>(DC)) {
    if (const auto* TD = RD->getDescribedClassTemplate()) {
      return TD->getTemplateParameters();
    }
  } else if (const auto* VTPSD =
                 dyn_cast<VarTemplatePartialSpecializationDecl>(DC)) {
    return VTPSD->getTemplateParameters();
  } else if (const auto* VD = dyn_cast<VarDecl>(DC)) {
    if (const auto* TD = VD->getDescribedVarTemplate()) {
      return TD->getTemplateParameters();
    }
  } else if (const auto* AD = dyn_cast<TypeAliasDecl>(DC)) {
    if (const auto* TD = AD->getDescribedAliasTemplate()) {
      return TD->getTemplateParameters();
    }
  }
  return nullptr;
}

/// Make sure `DC` won't cause Sema::LookupQualifiedName to fail an assertion.
bool IsContextSafeForLookup(const clang::DeclContext* DC) {
  if (const auto* TD = dyn_cast<clang::TagDecl>(DC)) {
    return TD->isCompleteDefinition() || TD->isBeingDefined();
  }
  return DC->isDependentContext() || !isa<clang::LinkageSpecDecl>(DC);
}

/// \brief Returns whether `Ctor` would override the in-class initializer for
/// `Field`.
bool ConstructorOverridesInitializer(const clang::CXXConstructorDecl* Ctor,
                                     const clang::FieldDecl* Field) {
  const clang::FunctionDecl* CtorDefn = nullptr;
  if (Ctor->isDefined(CtorDefn) &&
      (Ctor = dyn_cast_or_null<CXXConstructorDecl>(CtorDefn))) {
    for (const auto* Init : Ctor->inits()) {
      if (Init->getMember() == Field && !Init->isInClassMemberInitializer()) {
        return true;
      }
    }
  }
  return false;
}

/// \return true if `D` should not be visited because its name will never be
/// uttered due to aliasing rules.
bool SkipAliasedDecl(const clang::Decl* D) {
  return absl::GetFlag(FLAGS_experimental_alias_template_instantiations) &&
         (FindSpecializedTemplate(D) != D);
}

bool IsObjCForwardDecl(const clang::ObjCInterfaceDecl* decl) {
  return !decl->isThisDeclarationADefinition();
}

const clang::Decl* FindImplicitDeclForStmt(
    const IndexedParentMap* AllParents, const clang::Stmt* Stmt,
    llvm::SmallVector<unsigned, 16>* StmtPath) {
  for (const auto& Current : RootTraversal(AllParents, Stmt)) {
    if (Current.decl && Current.decl->isImplicit() &&
        !isa<VarDecl>(Current.decl) &&
        !(isa<CXXRecordDecl>(Current.decl) &&
          dyn_cast<CXXRecordDecl>(Current.decl)->isLambda())) {
      // If this is an implicit variable declaration, we assume that it is one
      // of the implicit declarations attached to a range for loop. We ignore
      // its implicitness, which lets us associate a source location with the
      // implicit references to 'begin', 'end' and operators. We also skip
      // the implicit CXXRecordDecl that's created for lambda expressions,
      // since it may include interesting non-implicit nodes.
      return Current.decl;
    }
    if (Current.indexed_parent && StmtPath) {
      StmtPath->push_back(Current.indexed_parent->index);
    }
  }
  return nullptr;
}

template <typename T, typename... Tail>
std::string DumpString(const T& val, Tail&&... tail) {
  std::string s;
  llvm::raw_string_ostream ss(s);
  val.dump(ss, std::forward<Tail>(tail)...);
  return s;
}

bool IsCompleteAggregateType(clang::QualType Type) {
  if (Type.isNull()) {
    return false;
  }
  Type = Type.getCanonicalType();
  return !Type->isIncompleteType() && Type->isAggregateType();
}

llvm::SmallVector<const clang::Decl*, 5> GetInitExprDecls(
    const clang::InitListExpr* ILE) {
  CHECK(ILE->isSemanticForm());
  QualType Type = ILE->getType();
  // Ignore non-aggregate and copy initialization.
  if (!IsCompleteAggregateType(Type) || ILE->isTransparent()) {
    return {};
  }
  if (const clang::FieldDecl* Field = ILE->getInitializedFieldInUnion()) {
    return {Field};
  }
  llvm::SmallVector<const clang::Decl*, 5> result;
  if (const auto* Decl = Type->getAsCXXRecordDecl();
      Decl && (Decl = Decl->getDefinition())) {
    for (const auto& Base : Decl->bases()) {
      result.push_back(CHECK_NOTNULL(Base.getType()->getAsTagDecl()));
    }
  }
  if (const auto* Decl = Type->getAsRecordDecl();
      Decl && (Decl = Decl->getDefinition())) {
    for (const auto* Field : Decl->fields()) {
      if (!Field->isUnnamedBitfield()) {
        result.push_back(Field);
      }
    }
  }
  return result;
}

clang::InitListExpr* GetSemanticForm(clang::InitListExpr* ILE) {
  return (ILE->isSemanticForm() ? ILE : ILE->getSemanticForm());
}

clang::InitListExpr* GetSyntacticForm(clang::InitListExpr* ILE) {
  return (ILE->isSyntacticForm() ? ILE : ILE->getSyntacticForm());
}

const clang::InitListExpr* GetSyntacticForm(const clang::InitListExpr* ILE) {
  return (ILE->isSyntacticForm() ? ILE : ILE->getSyntacticForm());
}

}  // anonymous namespace

bool IsClaimableForTraverse(const clang::Decl* decl) {
  // Operationally, we'll define this as any decl that causes
  // Job->UnderneathImplicitTemplateInstantiation to be set.
  if (auto* VTSD = dyn_cast<const clang::VarTemplateSpecializationDecl>(decl)) {
    return !VTSD->isExplicitInstantiationOrSpecialization();
  }
  if (auto* CTSD =
          dyn_cast<const clang::ClassTemplateSpecializationDecl>(decl)) {
    return !CTSD->isExplicitInstantiationOrSpecialization();
  }
  if (auto* FD = dyn_cast<const clang::FunctionDecl>(decl)) {
    if (const auto* MSI = FD->getMemberSpecializationInfo()) {
      // The definitions of class template member functions are not necessarily
      // dominated by the class template definition.
      if (!MSI->isExplicitSpecialization()) {
        return true;
      }
    } else if (const auto* FSI = FD->getTemplateSpecializationInfo()) {
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
  PruneCheck(IndexerASTVisitor* visitor, const clang::Decl* decl)
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
      if (absl::GetFlag(FLAGS_experimental_threaded_claiming) ||
          !visitor_->Observer.claimImplicitNode(cleanup_id_)) {
        can_prune_ = Prunability::kDeferred;
      }
    } else if (llvm::isa<clang::ClassTemplateSpecializationDecl>(decl)) {
      GenerateCleanupId(decl);
      if (absl::GetFlag(FLAGS_experimental_threaded_claiming) ||
          !visitor_->Observer.claimImplicitNode(cleanup_id_)) {
        can_prune_ = Prunability::kDeferIncompleteFunctions;
      }
    }
  }
  ~PruneCheck() {
    if (!absl::GetFlag(FLAGS_experimental_threaded_claiming) &&
        !cleanup_id_.empty()) {
      visitor_->Observer.finishImplicitNode(cleanup_id_);
    }
  }

  /// \return true if the decl supplied during construction doesn't need
  /// traversed.
  Prunability can_prune() const { return can_prune_; }

  const std::string& cleanup_id() { return cleanup_id_; }

 private:
  void GenerateCleanupId(const clang::Decl* decl) {
    // TODO(zarko): Check to see if non-function members of a class
    // can be traversed once per argument set.
    cleanup_id_ =
        absl::StrCat(visitor_->BuildNodeIdForDecl(decl).getRawIdentity(), "#",
                     visitor_->getGraphObserver().getBuildConfig());
    // It's critical that we distinguish between different argument lists
    // here even if aliasing is turned on; otherwise we will drop data.
    // If aliasing is off, the NodeId already contains this information.
    if (absl::GetFlag(FLAGS_experimental_alias_template_instantiations)) {
      llvm::raw_string_ostream ostream(cleanup_id_);
      for (const auto& Current :
           RootTraversal(visitor_->getAllParents(), decl)) {
        if (!(Current.decl && Current.indexed_parent)) continue;

        if (const auto* czdecl =
                dyn_cast<ClassTemplateSpecializationDecl>(Current.decl)) {
          ostream << "#"
                  << HashToString(visitor_->Hash(
                         &czdecl->getTemplateInstantiationArgs()));
        } else if (const auto* fdecl = dyn_cast<FunctionDecl>(Current.decl)) {
          if (const auto* template_args =
                  fdecl->getTemplateSpecializationArgs()) {
            ostream << "#" << HashToString(visitor_->Hash(template_args));
          }
        } else if (const auto* vdecl =
                       dyn_cast<VarTemplateSpecializationDecl>(Current.decl)) {
          ostream << "#"
                  << HashToString(visitor_->Hash(
                         &vdecl->getTemplateInstantiationArgs()));
        }
      }
    }
    cleanup_id_ = CompressString(cleanup_id_);
  }
  Prunability can_prune_ = Prunability::kNone;
  std::string cleanup_id_;
  IndexerASTVisitor* visitor_;
};

const IndexedParentMap* IndexerASTVisitor::getAllParents() {
  if (!AllParents) {
    // We always need to run over the whole translation unit, as
    // hasAncestor can escape any subtree.
    // TODO(zarko): Is this relavant for naming?
    ProfileBlock block(Observer.getProfilingCallback(), "build_parent_map");
    AllParents = absl::make_unique<IndexedParentMap>(
        IndexedParentMap::Build(Context.getTranslationUnitDecl()));
  }
  return AllParents.get();
}

const IndexedParent* IndexerASTVisitor::getIndexedParent(
    const ast_type_traits::DynTypedNode& Node) {
  return getAllParents()->GetIndexedParent(Node);
}

bool IndexerASTVisitor::declDominatesPrunableSubtree(const clang::Decl* Decl) {
  return getAllParents()->DeclDominatesPrunableSubtree(Decl);
}

bool IndexerASTVisitor::IsDefinition(const clang::VarDecl* VD) {
  if (const auto* PVD = dyn_cast<ParmVarDecl>(VD)) {
    // For parameters, we want to report them as definitions iff they're
    // part of a function definition.  Current (2013-02-14) Clang appears
    // to report all function parameter declarations as definitions.
    if (const auto* FD = dyn_cast<FunctionDecl>(PVD->getDeclContext())) {
      return IsDefinition(FD);
    }
  }
  // This one's a little quirky.  It would actually work to just return
  // implicit_cast<bool>(VD->isThisDeclarationADefinition()), because
  // VarDecl::DeclarationOnly is zero, but this is more explicit.
  return VD->isThisDeclarationADefinition() != VarDecl::DeclarationOnly;
}

clang::SourceRange IndexerASTVisitor::RangeForNameOfDeclaration(
    const clang::NamedDecl* Decl) const {
  return ClangRangeFinder(Observer.getSourceManager(),
                          Observer.getLangOptions())
      .RangeForNameOf(Decl);
}

void IndexerASTVisitor::MaybeRecordDefinitionRange(
    const absl::optional<GraphObserver::Range>& R,
    const GraphObserver::NodeId& Id,
    const absl::optional<GraphObserver::NodeId>& DeclId) {
  if (R) {
    Observer.recordDefinitionBindingRange(R.value(), Id, DeclId);
  }
}

void IndexerASTVisitor::MaybeRecordFullDefinitionRange(
    const absl::optional<GraphObserver::Range>& R,
    const GraphObserver::NodeId& Id,
    const absl::optional<GraphObserver::NodeId>& DeclId) {
  if (R) {
    Observer.recordFullDefinitionRange(R.value(), Id, DeclId);
  }
}

GraphObserver::Implicit IndexerASTVisitor::IsImplicit(
    const GraphObserver::Range& Range) {
  return (Job->UnderneathImplicitTemplateInstantiation ||
          Range.Kind != GraphObserver::Range::RangeKind::Physical)
             ? GraphObserver::Implicit::Yes
             : GraphObserver::Implicit::No;
}

void IndexerASTVisitor::RecordCallEdges(const GraphObserver::Range& Range,
                                        const GraphObserver::NodeId& Callee) {
  if (Job->BlameStack.empty()) {
    if (auto FileId = Observer.recordFileInitializer(Range)) {
      Observer.recordCallEdge(Range, FileId.value(), Callee, IsImplicit(Range));
    }
  } else {
    for (const auto& Caller : Job->BlameStack.back()) {
      Observer.recordCallEdge(Range, Caller, Callee, IsImplicit(Range));
    }
  }
}

/// An in-flight possible lookup result used to approximate qualified lookup.
struct PossibleLookup {
  clang::LookupResult Result;
  const clang::DeclContext* Context;
};

/// Greedily consumes tokens to try and find the longest qualified name that
/// results in a nonempty lookup result.
class PossibleLookups {
 public:
  /// \param RC The most specific context to start searching in.
  /// \param TC The translation unit context.
  PossibleLookups(clang::ASTContext& C, clang::Sema& S,
                  const clang::DeclContext* RC, const clang::DeclContext* TC)
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
  LookupResult AdvanceLookup(const clang::Token& Tok) {
    if (Tok.is(clang::tok::coloncolon)) {
      // This flag only matters if the possible lookup list is empty.
      // Just drop ::s on the floor otherwise.
      ForceTUContext = true;
      return LookupResult::Progress;
    } else if (Tok.is(clang::tok::raw_identifier)) {
      bool Invalid = false;
      const char* TokChar = Context.getSourceManager().getCharacterData(
          Tok.getLocation(), &Invalid);
      if (Invalid || !TokChar) {
        return LookupResult::Done;
      }
      llvm::StringRef TokText(TokChar, Tok.getLength());
      const auto& Id = Context.Idents.get(TokText);
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
  const std::vector<PossibleLookup>& LookupState() { return Lookups; }

 private:
  /// Start a new lookup from `DeclName`.
  LookupResult StartLookups(const clang::DeclarationNameInfo& DeclName) {
    CHECK(Lookups.empty());
    const clang::DeclContext* Context =
        ForceTUContext ? TUContext : RootContext;
    do {
      clang::LookupResult FirstLookup(
          Sema, DeclName, clang::Sema::LookupNameKind::LookupAnyName);
      FirstLookup.suppressDiagnostics();
      if (IsContextSafeForLookup(Context) &&
          Sema.LookupQualifiedName(
              FirstLookup, const_cast<clang::DeclContext*>(Context), false)) {
        Lookups.push_back({std::move(FirstLookup), Context});
      } else {
        CHECK(FirstLookup.empty());
        // We could be looking at a (type) parameter here. Note that this
        // may still be part of a qualified (dependent) name.
        if (const auto* FunctionContext =
                dyn_cast_or_null<clang::FunctionDecl>(Context)) {
          for (const auto& Param : FunctionContext->parameters()) {
            if (Param->getDeclName() == DeclName.getName()) {
              clang::LookupResult DerivedResult(clang::LookupResult::Temporary,
                                                FirstLookup);
              DerivedResult.suppressDiagnostics();
              DerivedResult.addDecl(Param);
              Lookups.push_back({std::move(DerivedResult), Context});
            }
          }
        } else if (const auto* TemplateParams = GetTypeParameters(Context)) {
          for (const auto& TParam : *TemplateParams) {
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
  LookupResult AdvanceLookups(const clang::DeclarationNameInfo& DeclName) {
    // Try to advance each lookup we have stored.
    std::vector<PossibleLookup> ResultLookups;
    for (auto& Lookup : Lookups) {
      for (const clang::NamedDecl* Result : Lookup.Result) {
        const auto* Context = dyn_cast<clang::DeclContext>(Result);
        if (!Context) {
          Context = Lookup.Context;
        }
        clang::LookupResult NextResult(
            Sema, DeclName, clang::Sema::LookupNameKind::LookupAnyName);
        NextResult.suppressDiagnostics();
        if (IsContextSafeForLookup(Context) &&
            Sema.LookupQualifiedName(
                NextResult, const_cast<clang::DeclContext*>(Context), false)) {
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
  clang::ASTContext& Context;
  /// The Sema instance we're using.
  clang::Sema& Sema;
  /// The nearest declaration context.
  const clang::DeclContext* RootContext;
  /// The translation unit's declaration context.
  const clang::DeclContext* TUContext;
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
void InsertAnchorMarks(std::string& Text, std::vector<MiniAnchor>& Anchors) {
  // Drop empty or negative-length anchors.
  Anchors.erase(
      std::remove_if(Anchors.begin(), Anchors.end(),
                     [](const MiniAnchor& A) { return A.Begin >= A.End; }),
      Anchors.end());
  std::sort(Anchors.begin(), Anchors.end(),
            [](const MiniAnchor& A, const MiniAnchor& B) {
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
    clang::FileID Id, const GraphObserver::NodeId& FileNode) {
  const auto& RCL = Context.Comments;
  // Find the block of comments for the given file. This behavior is not well-
  // defined by Clang, which commits only to the RawComments being
  // "sorted in order of appearance in the translation unit".
  const std::map<unsigned, RawComment*>* FileComments =
      RCL.getCommentsInFile(Id);
  if (FileComments == nullptr || FileComments->empty()) {
    return;
  }
  // Find the first RawComment whose start location is greater or equal to
  // the start of the file whose FileID is Id.
  RawComment* C = FileComments->begin()->second;
  // Walk through the comments in Id starting with the one at the top. If we
  // ever leave Id, then we're done. (The first time around the loop, if C isn't
  // already in Id, this check will immediately break;.)
  //
  // Here's a simple heuristic: the first comment in a file is the file-level
  // comment. This is bad for files with (e.g.) license blocks, but we can
  // gradually refine as necessary.
  if (VisitedComments.find(C) == VisitedComments.end()) {
    VisitComment(C, Context.getTranslationUnitDecl(), FileNode);
  }
}

void IndexerASTVisitor::VisitComment(
    const clang::RawComment* Comment, const clang::DeclContext* DC,
    const GraphObserver::NodeId& DocumentedNode) {
  VisitedComments.insert(Comment);
  const auto& SM = *Observer.getSourceManager();
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
  auto RecordDocDeclUse = [&](const clang::SourceRange& Range,
                              const GraphObserver::NodeId& ResultId) {
    if (auto RCC = ExplicitRangeInCurrentContext(Range)) {
      Observer.recordDeclUseLocationInDocumentation(RCC.value(), ResultId);
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
    for (const auto& Results : Lookups.LookupState()) {
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
    for (const auto& Tok : IdentifierTokens) {
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
        {Tok.getLocation(), StrippedRawText.size()});
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
  for (const auto& Anchor : StrippedRawTextAnchors) {
    LinkNodes.push_back(Anchor.AnchoredTo);
  }
  // Attribute the raw comment text to its associated decl.
  if (auto RCC = ExplicitRangeInCurrentContext(Comment->getSourceRange())) {
    Observer.recordDocumentationRange(RCC.value(), DocumentedNode);
    Observer.recordDocumentationText(DocumentedNode, StrippedRawText,
                                     LinkNodes);
  }
}

void IndexerASTVisitor::VisitAttributes(
    const clang::Decl* Decl, const GraphObserver::NodeId& TargetNode) {
  for (const auto& Attr : Decl->attrs()) {
    if (const auto* DepAttr = clang::dyn_cast<clang::DeprecatedAttr>(Attr)) {
      Observer.recordDeprecated(TargetNode, DepAttr->getMessage());
    }
  }
}

bool IndexerASTVisitor::VisitDecl(const clang::Decl* Decl) {
  if ((Job->UnderneathImplicitTemplateInstantiation || Decl == nullptr) &&
      !Decl->hasAttrs()) {
    // Template instantiation can't add any documentation text.
    return true;
  }
  const auto* CommentOrNull = Context.getRawCommentForDeclNoCache(Decl);
  if (!CommentOrNull && !Decl->hasAttrs()) {
    // Fast path: if there are no attached documentation comments or attributes,
    // bail.
    return true;
  }
  const auto* DCxt = dyn_cast<DeclContext>(Decl);
  if (!DCxt) {
    DCxt = Decl->getDeclContext();
    if (!DCxt) {
      DCxt = Context.getTranslationUnitDecl();
    }
  }

  if (const auto* DC = dyn_cast_or_null<DeclContext>(Decl)) {
    if (auto DCID = BuildNodeIdForDeclContext(DC)) {
      VisitAttributes(Decl, DCID.value());
      if (CommentOrNull == nullptr) {
        return true;
      }
      if (const auto* IFaceDecl = dyn_cast_or_null<ObjCInterfaceDecl>(DC)) {
        VisitObjCInterfaceDeclComment(IFaceDecl, CommentOrNull, DCxt, DCID);
      } else if (const auto* R = dyn_cast_or_null<RecordDecl>(DC)) {
        VisitRecordDeclComment(R, CommentOrNull, DCxt, DCID);
      } else {
        VisitComment(CommentOrNull, DCxt, DCID.value());
      }
    }
    if (const auto* CTPSD =
            dyn_cast_or_null<ClassTemplatePartialSpecializationDecl>(Decl)) {
      auto NodeId = BuildNodeIdForDecl(CTPSD);
      VisitAttributes(Decl, NodeId);
      if (CommentOrNull != nullptr) VisitComment(CommentOrNull, DCxt, NodeId);
    }
    if (const auto* FD = dyn_cast_or_null<FunctionDecl>(Decl)) {
      if (const auto* FTD = FD->getDescribedFunctionTemplate()) {
        auto NodeId = BuildNodeIdForDecl(FTD);
        VisitAttributes(Decl, NodeId);
        if (CommentOrNull != nullptr) VisitComment(CommentOrNull, DCxt, NodeId);
      }
    }
  } else {
    if (const auto* VD = dyn_cast_or_null<VarDecl>(Decl)) {
      if (const auto* VTD = VD->getDescribedVarTemplate()) {
        auto NodeId = BuildNodeIdForDecl(VTD);
        VisitAttributes(VTD, NodeId);
        if (CommentOrNull != nullptr) VisitComment(CommentOrNull, DCxt, NodeId);
      }
    } else if (const auto* AD = dyn_cast_or_null<TypeAliasDecl>(Decl)) {
      if (const auto* TATD = AD->getDescribedAliasTemplate()) {
        auto NodeId = BuildNodeIdForDecl(TATD);
        VisitAttributes(TATD, NodeId);
        if (CommentOrNull != nullptr) VisitComment(CommentOrNull, DCxt, NodeId);
      }
    }
    auto NodeId = BuildNodeIdForDecl(Decl);
    VisitAttributes(Decl, NodeId);
    if (CommentOrNull != nullptr) VisitComment(CommentOrNull, DCxt, NodeId);
  }
  return true;
}

void IndexerASTVisitor::VisitObjCInterfaceDeclComment(
    const ObjCInterfaceDecl* Decl, const RawComment* Comment,
    const DeclContext* DCxt, absl::optional<GraphObserver::NodeId> DCID) {
  // Don't record comments for ObjC class forward declarations because
  // their comments generally aren't useful documentation about the class,
  // they are more likely to be about why a forward declaration was used
  // or be totally unrelated to the class.
  if (shouldEmitObjCForwardClassDeclDocumentation() ||
      !IsObjCForwardDecl(Decl)) {
    VisitComment(Comment, DCxt, DCID.value());
  }
}

void IndexerASTVisitor::VisitRecordDeclComment(
    const RecordDecl* Decl, const RawComment* Comment, const DeclContext* DCxt,
    absl::optional<GraphObserver::NodeId> DCID) {
  // Don't record comments for forward declarations because their comments
  // generally aren't useful documentation about the class, they are more likely
  // to be about why a forward declaration was used or be totally unrelated to
  // the class.
  if (shouldEmitCppForwardDeclDocumentation() ||
      Decl->getDefinition() == Decl) {
    VisitComment(Comment, DCxt, DCID.value());
  }
}

bool IndexerASTVisitor::TraverseCXXConstructorDecl(
    clang::CXXConstructorDecl* CD) {
  auto DNI = CD->getNameInfo();
  if (DNI.getNamedTypeInfo() == nullptr && !CD->isImplicit()) {
    // Clang does not currently set the NamedTypeInfo for constructors,
    // but does for destructors and conversion operators.  As such, work
    // around this here. Implicit constructors do not get TypeSourceInfo.
    DNI.setNamedTypeInfo(Context.getTrivialTypeSourceInfo(
        QualType(CD->getParent()->getTypeForDecl(), 0), CD->getLocation()));
  }
  return RecursiveASTVisitor::TraverseCXXConstructorDecl(CD) &&
         TraverseDeclarationNameInfo(DNI);
}

bool IndexerASTVisitor::TraverseDecl(clang::Decl* Decl) {
  if (ShouldStopIndexing()) {
    return false;
  }
  if (Decl == nullptr) {
    return true;
  }

  auto Scope = RestoreValue(Job->PruneIncompleteFunctions);
  if (Job->PruneIncompleteFunctions) {
    if (auto* FD = llvm::dyn_cast<clang::FunctionDecl>(Decl)) {
      if (!FD->isThisDeclarationADefinition()) {
        return true;
      }
    }
  }
  if (absl::GetFlag(FLAGS_experimental_threaded_claiming)) {
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
  if (auto* FD = dyn_cast_or_null<clang::FunctionDecl>(Decl)) {
    if (unsigned BuiltinID = FD->getBuiltinID()) {
      if (!Observer.getPreprocessor()->getBuiltinInfo().isPredefinedLibFunction(
              BuiltinID)) {
        // These builtins act weirdly (by, e.g., defining themselves inside the
        // macro context where they first appeared, but then also in the global
        // namespace). Don't traverse them.
        return true;
      }
    }
    auto Scope = std::make_tuple(RestoreStack(Job->BlameStack),
                                 RestoreStack(Job->RangeContext));

    if (FD->isTemplateInstantiation() &&
        FD->getTemplateSpecializationKind() !=
            clang::TSK_ExplicitSpecialization) {
      // Explicit specializations have ranges.
      if (const auto RangeId = BuildNodeIdForRefToDeclContext(FD)) {
        Job->RangeContext.push_back(RangeId.value());
      } else {
        Job->RangeContext.push_back(BuildNodeIdForDecl(FD));
      }
    }
    if (const auto BlameId = BuildNodeIdForDeclContext(FD)) {
      Job->BlameStack.push_back(IndexJob::SomeNodes(1, BlameId.value()));
    } else {
      Job->BlameStack.push_back(
          IndexJob::SomeNodes(1, BuildNodeIdForRefToDecl(FD)));
    }

    // Dispatch the remaining logic to the base class TraverseDecl() which will
    // call TraverseX(X*) for the most-derived X.
    return Base::TraverseDecl(FD);
  } else if (auto* ID = dyn_cast_or_null<clang::FieldDecl>(Decl)) {
    // This will also cover the case of clang::ObjCIVarDecl since it is a
    // subclass.
    if (ID->hasInClassInitializer()) {
      // Blame calls from in-class initializers I on all ctors C of the
      // containing class so long as C does not have its own initializer for
      // I's field.
      auto R = RestoreStack(Job->BlameStack);
      if (auto* CR = dyn_cast_or_null<clang::CXXRecordDecl>(ID->getParent())) {
        if ((CR = CR->getDefinition())) {
          IndexJob::SomeNodes Ctors;
          auto TryCtor = [&](const clang::CXXConstructorDecl* Ctor) {
            if (!ConstructorOverridesInitializer(Ctor, ID)) {
              if (const auto BlameId = BuildNodeIdForRefToDeclContext(Ctor)) {
                Ctors.push_back(BlameId.value());
              }
            }
          };
          for (const auto* SubDecl : CR->decls()) {
            if (const auto* Ctor =
                    dyn_cast_or_null<CXXConstructorDecl>(SubDecl)) {
              TryCtor(Ctor);
            } else if (const auto* Templ =
                           dyn_cast_or_null<FunctionTemplateDecl>(SubDecl)) {
              if (const auto* Ctor = dyn_cast_or_null<CXXConstructorDecl>(
                      Templ->getTemplatedDecl())) {
                TryCtor(Ctor);
              }
              for (const auto* Spec : Templ->specializations()) {
                if (const auto* Ctor =
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
      return Base::TraverseDecl(ID);
    }
  } else if (auto* MD = dyn_cast_or_null<clang::ObjCMethodDecl>(Decl)) {
    // These variables (R and S) clean up the stacks (Job->BlameStack and
    // Job->RangeContext) when the local variables (R and S) are
    // destructed.
    auto Scope = std::make_tuple(RestoreStack(Job->BlameStack),
                                 RestoreStack(Job->RangeContext));

    if (const auto BlameId = BuildNodeIdForDeclContext(MD)) {
      Job->BlameStack.push_back(IndexJob::SomeNodes(1, BlameId.value()));
    } else {
      Job->BlameStack.push_back(IndexJob::SomeNodes(1, BuildNodeIdForDecl(MD)));
    }

    // Dispatch the remaining logic to the base class TraverseDecl() which will
    // call TraverseX(X*) for the most-derived X.
    return Base::TraverseDecl(MD);
  }

  return Base::TraverseDecl(Decl);
}

bool IndexerASTVisitor::VisitCXXDependentScopeMemberExpr(
    const clang::CXXDependentScopeMemberExpr* E) {
  absl::optional<GraphObserver::NodeId> Root = absl::nullopt;
  auto BaseType = E->getBaseType();
  if (BaseType.getTypePtrOrNull()) {
    auto* Builtin = dyn_cast<BuiltinType>(BaseType.getTypePtr());
    // The "Dependent" builtin type is not useful, so we'll keep this lookup
    // rootless. We could alternately invent a singleton type for the range of
    // the lhs expression.
    if (!Builtin || Builtin->getKind() != BuiltinType::Dependent) {
      Root = BuildNodeIdForType(BaseType);
    }
  }

  if (auto DepNodeId = RecordEdgesForDependentName(
          E->getQualifierLoc(), E->getMember(), E->getMemberLoc(), Root)) {
    if (auto RCC =
            ExplicitRangeInCurrentContext(NormalizeRange(E->getMemberLoc()))) {
      Observer.recordDeclUseLocation(*RCC, *DepNodeId,
                                     GraphObserver::Claimability::Claimable,
                                     IsImplicit(*RCC));
    }
    if (E->hasExplicitTemplateArgs()) {
      if (auto ArgIds = BuildTemplateArgumentList(E->template_arguments())) {
        auto TappNodeId = Observer.recordTappNode(*DepNodeId, *ArgIds);
        auto StmtId = BuildNodeIdForImplicitStmt(E);
        auto Range = NormalizeRange({E->getMemberLoc(), E->getEndLoc()});
        if (auto RCC = RangeInCurrentContext(StmtId, Range)) {
          Observer.recordDeclUseLocation(
              RCC.value(), TappNodeId, GraphObserver::Claimability::Unclaimable,
              IsImplicit(RCC.value()));
        }
      }
    }
  }
  return true;
}

bool IndexerASTVisitor::VisitMemberExpr(const clang::MemberExpr* E) {
  if (E->getMemberLoc().isInvalid()) {
    return true;
  }
  for (const auto* Child : E->children()) {
    if (const auto* DeclRef = dyn_cast<clang::DeclRefExpr>(Child)) {
      if (isa<clang::DecompositionDecl>(DeclRef->getDecl())) {
        // Ignore field references synthesized from structured bindings, as
        // they are just a clang implementation detail. Skip the references.
        return true;
      }
    }
  }
  if (const auto* FieldDecl = E->getMemberDecl()) {
    auto Range = NormalizeRange(E->getMemberLoc());
    auto StmtId = BuildNodeIdForImplicitStmt(E);
    if (auto RCC = RangeInCurrentContext(StmtId, Range)) {
      Observer.recordDeclUseLocation(
          RCC.value(), BuildNodeIdForRefToDecl(FieldDecl),
          GraphObserver::Claimability::Unclaimable, IsImplicit(RCC.value()));
      if (E->hasExplicitTemplateArgs()) {
        // We still want to link the template args.
        BuildTemplateArgumentList(E->template_arguments());
      }
    }
  }
  return true;
}

bool IndexerASTVisitor::IndexConstructExpr(const clang::CXXConstructExpr* E,
                                           const clang::TypeSourceInfo* TSI) {
  if (const auto* Callee = E->getConstructor()) {
    // Clang doesn't invoke VisitDeclRefExpr on constructors, so we
    // must do so manually.
    // TODO(zarko): What about static initializers? Do we blame these on the
    // translation unit?
    clang::SourceLocation RPL = E->getParenOrBraceRange().getEnd();
    clang::SourceLocation RefLoc = E->getBeginLoc();
    if (TSI != nullptr) {
      if (const auto ETL =
              TSI->getTypeLoc().getAs<clang::ElaboratedTypeLoc>()) {
        RefLoc = ETL.getNamedTypeLoc().getBeginLoc();
      }
    }

    // We assume that a constructor call was inserted by the compiler (or
    // is otherwise implicit) if it's being provided with arguments but it has
    // an invalid right-paren location, since such a call would be impossible
    // to write down.
    bool IsImplicit = !RPL.isValid() && E->getNumArgs() > 0;
    VisitDeclRefOrIvarRefExpr(E, Callee, RefLoc, IsImplicit);

    clang::SourceRange SR = E->getSourceRange();
    // If there are not arguments and no braces or parens, getEndLoc()
    // is the same as getLocation(), rather than the type.
    // TODO(shahms): Upstream this into CXXConstructExpr::getEndLoc.
    if (TSI != nullptr && SR.getBegin() == SR.getEnd()) {
      SR.setEnd(TSI->getTypeLoc().getEndLoc());
    }
    auto StmtId = BuildNodeIdForImplicitStmt(E);
    if (auto RCC = RangeInCurrentContext(StmtId, NormalizeRange(SR))) {
      RecordCallEdges(*RCC, BuildNodeIdForRefToDecl(Callee));
    }
  }
  // This is a hack which pairs with that from VisitRecordTypeLoc to emit ref/id
  // rather than ref for types visited as part of a constructor expr for that
  // record.
  if (TSI != nullptr) {
    clang::TypeLoc TL = TSI->getTypeLoc().getAsAdjusted<clang::RecordTypeLoc>();
    if (TL &&
        TL.getTypePtr() == E->getType()->getAsAdjusted<clang::RecordType>()) {
      if (auto RCC = ExpandedRangeInCurrentContext(TL.getSourceRange())) {
        if (auto Nodes = BuildNodeSetForType(TL.getTypePtr())) {
          Observer.recordTypeIdSpellingLocation(*RCC, Nodes.ForReference(),
                                                Nodes.claimability(),
                                                IsImplicit(*RCC));
        }
      }
    }
  }
  return true;
}

bool IndexerASTVisitor::TraverseConstructorInitializer(
    clang::CXXCtorInitializer* Init) {
  if (Init->isMemberInitializer()) {
    return Base::TraverseConstructorInitializer(Init);
  }
  if (const auto* CE = dyn_cast<clang::CXXConstructExpr>(Init->getInit())) {
    if (IndexConstructExpr(CE, Init->getTypeSourceInfo())) {
      auto Scope = PushScope(Job->ConstructorStack, CE);
      return Base::TraverseConstructorInitializer(Init);
    }
    return false;
  }
  return Base::TraverseConstructorInitializer(Init);
}

bool IndexerASTVisitor::VisitCXXConstructExpr(
    const clang::CXXConstructExpr* E) {
  // Skip Visiting CXXConstructExprs directly which were already
  // visited by a parent.
  auto iter = std::find(Job->ConstructorStack.crbegin(),
                        Job->ConstructorStack.crend(), E);
  if (iter != Job->ConstructorStack.crend()) {
    return true;
  }

  clang::TypeSourceInfo* TSI = nullptr;
  if (const auto* TE = dyn_cast<clang::CXXTemporaryObjectExpr>(E)) {
    TSI = TE->getTypeSourceInfo();
  }
  return IndexConstructExpr(E, TSI);
}

bool IndexerASTVisitor::VisitCXXDeleteExpr(const clang::CXXDeleteExpr* E) {
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
  absl::optional<GraphObserver::NodeId> DDId;
  if (const auto* CD = BaseType->getAsCXXRecordDecl()) {
    if (const auto* DD = CD->getDestructor()) {
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
    DDId = RecordEdgesForDependentName(clang::NestedNameSpecifierLoc(),
                                       DtorName, E->getBeginLoc(), TyId);
  }
  if (DDId) {
    clang::SourceRange SR = NormalizeRange(E->getSourceRange());
    auto StmtId = BuildNodeIdForImplicitStmt(E);
    if (auto RCC = RangeInCurrentContext(StmtId, SR)) {
      RecordCallEdges(RCC.value(), DDId.value());
    }
  }
  return true;
}

bool IndexerASTVisitor::TraverseCXXFunctionalCastExpr(
    clang::CXXFunctionalCastExpr* E) {
  if (const auto* CE = FindConstructExpr(E)) {
    if (IndexConstructExpr(CE, E->getTypeInfoAsWritten())) {
      auto Scope = PushScope(Job->ConstructorStack, CE);
      return Base::TraverseCXXFunctionalCastExpr(E);
    }
    return false;
  }
  return Base::TraverseCXXFunctionalCastExpr(E);
}

bool IndexerASTVisitor::TraverseCXXNewExpr(clang::CXXNewExpr* E) {
  if (const auto* CE = E->getConstructExpr()) {
    if (IndexConstructExpr(CE, E->getAllocatedTypeSourceInfo())) {
      auto Scope = PushScope(Job->ConstructorStack, CE);
      return Base::TraverseCXXNewExpr(E);
    }
    return false;
  }
  return Base::TraverseCXXNewExpr(E);
}

bool IndexerASTVisitor::TraverseCXXTemporaryObjectExpr(
    clang::CXXTemporaryObjectExpr* E) {
  if (E == nullptr) return true;
  if (IndexConstructExpr(E, E->getTypeSourceInfo())) {
    auto Scope = PushScope(Job->ConstructorStack, E);
    return Base::TraverseCXXTemporaryObjectExpr(E);
  }
  return false;
}

bool IndexerASTVisitor::VisitCXXNewExpr(const clang::CXXNewExpr* E) {
  auto StmtId = BuildNodeIdForImplicitStmt(E);
  if (FunctionDecl* New = E->getOperatorNew()) {
    auto NewId = BuildNodeIdForRefToDecl(New);
    clang::SourceLocation NewLoc = E->getBeginLoc();
    if (NewLoc.isFileID()) {
      clang::SourceRange NewRange = NormalizeRange(NewLoc);
      if (auto RCC = RangeInCurrentContext(StmtId, NewRange)) {
        Observer.recordDeclUseLocation(RCC.value(), NewId,
                                       GraphObserver::Claimability::Unclaimable,
                                       IsImplicit(RCC.value()));
      }
    }
  }
  return true;
}

bool IndexerASTVisitor::VisitCXXPseudoDestructorExpr(
    const clang::CXXPseudoDestructorExpr* E) {
  if (E->getDestroyedType().isNull()) {
    return true;
  }
  auto DTCan = E->getDestroyedType().getCanonicalType();
  if (DTCan.isNull() || !DTCan.isCanonical()) {
    return true;
  }
  absl::optional<GraphObserver::NodeId> TyId;
  clang::NestedNameSpecifierLoc NNSLoc;
  if (E->getDestroyedTypeInfo() != nullptr) {
    TyId = BuildNodeIdForType(E->getDestroyedTypeInfo()->getTypeLoc());
  } else if (E->hasQualifier()) {
    NNSLoc = E->getQualifierLoc();
  }
  auto DtorName = Context.DeclarationNames.getCXXDestructorName(
      CanQualType::CreateUnsafe(DTCan));
  if (auto DDId = RecordEdgesForDependentName(NNSLoc, DtorName,
                                              E->getTildeLoc(), TyId)) {
    if (auto RCC =
            ExplicitRangeInCurrentContext(NormalizeRange(E->getTildeLoc()))) {
      Observer.recordDeclUseLocation(*RCC, *DDId,
                                     GraphObserver::Claimability::Claimable,
                                     IsImplicit(*RCC));
    }
    clang::SourceRange SR = NormalizeRange(E->getSourceRange());
    auto StmtId = BuildNodeIdForImplicitStmt(E);
    if (auto RCC = RangeInCurrentContext(StmtId, SR)) {
      RecordCallEdges(RCC.value(), DDId.value());
    }
  }
  return true;
}

bool IndexerASTVisitor::VisitCXXUnresolvedConstructExpr(
    const clang::CXXUnresolvedConstructExpr* E) {
  auto QTCan = E->getTypeAsWritten().getCanonicalType();
  if (QTCan.isNull()) {
    return true;
  }
  CHECK(E->getTypeSourceInfo() != nullptr);
  auto TyId = BuildNodeIdForType(E->getTypeSourceInfo()->getTypeLoc());
  if (!TyId) {
    return true;
  }
  auto CtorName = Context.DeclarationNames.getCXXConstructorName(
      CanQualType::CreateUnsafe(QTCan));
  if (auto LookupId = RecordEdgesForDependentName(
          clang::NestedNameSpecifierLoc(), CtorName, E->getBeginLoc(), TyId)) {
    clang::SourceRange SR = NormalizeRange(E->getSourceRange());
    auto StmtId = BuildNodeIdForImplicitStmt(E);
    if (auto RCC = RangeInCurrentContext(StmtId, SR)) {
      RecordCallEdges(RCC.value(), LookupId.value());
    }
  }
  return true;
}

bool IndexerASTVisitor::VisitCallExpr(const clang::CallExpr* E) {
  clang::SourceRange SR = NormalizeRange(E->getSourceRange());
  auto StmtId = BuildNodeIdForImplicitStmt(E);
  if (auto RCC = RangeInCurrentContext(StmtId, SR)) {
    if (const auto* Callee = E->getCalleeDecl()) {
      auto CalleeId = BuildNodeIdForRefToDecl(Callee);
      RecordCallEdges(RCC.value(), CalleeId);
      for (const auto& S : Supports) {
        S->InspectCallExpr(*this, E, RCC.value(), CalleeId);
      }
    } else if (const auto* CE = E->getCallee()) {
      if (auto CalleeId = BuildNodeIdForExpr(CE, EmitRanges::Yes)) {
        RecordCallEdges(RCC.value(), CalleeId.value());
      }
    }
  }
  return true;
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForImplicitTemplateInstantiation(
    const clang::Decl* Decl) {
  if (const auto* FD = dyn_cast<const clang::FunctionDecl>(Decl)) {
    return BuildNodeIdForImplicitFunctionTemplateInstantiation(FD);
  } else if (const auto* CD =
                 dyn_cast<const clang::ClassTemplateSpecializationDecl>(Decl)) {
    return BuildNodeIdForImplicitClassTemplateInstantiation(CD);
  }
  // TODO(zarko): other template kinds as needed.
  return absl::nullopt;
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForImplicitClassTemplateInstantiation(
    const clang::ClassTemplateSpecializationDecl* CTSD) {
  if (CTSD->isExplicitInstantiationOrSpecialization()) {
    return absl::nullopt;
  }
  const clang::ASTTemplateArgumentListInfo* ArgsAsWritten = nullptr;
  if (const auto* CTPSD =
          dyn_cast<const clang::ClassTemplatePartialSpecializationDecl>(CTSD)) {
    ArgsAsWritten = CTPSD->getTemplateArgsAsWritten();
  }
  auto PrimaryOrPartial = CTSD->getSpecializedTemplateOrPartial();
  if (auto NIDS =
          (ArgsAsWritten ? BuildTemplateArgumentList(ArgsAsWritten->arguments())
                         : BuildTemplateArgumentList(
                               CTSD->getTemplateArgs().asArray()))) {
    if (auto SpecializedNode = BuildNodeIdForTemplateName(
            clang::TemplateName(CTSD->getSpecializedTemplate()))) {
      if (PrimaryOrPartial.is<clang::ClassTemplateDecl*>() &&
          !isa<const clang::ClassTemplatePartialSpecializationDecl>(CTSD)) {
        // Point to a tapp of the primary template.
        return Observer.recordTappNode(*SpecializedNode, *NIDS);
      }
    }
  }
  if (const auto* Partial =
          PrimaryOrPartial
              .dyn_cast<clang::ClassTemplatePartialSpecializationDecl*>()) {
    if (auto NIDS = BuildTemplateArgumentList(
            CTSD->getTemplateInstantiationArgs().asArray())) {
      // Point to a tapp of the partial template specialization.
      return Observer.recordTappNode(BuildNodeIdForDecl(Partial), *NIDS);
    }
  }
  return absl::nullopt;
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForImplicitFunctionTemplateInstantiation(
    const clang::FunctionDecl* FD) {
  std::vector<GraphObserver::NodeId> NIDS;
  const clang::TemplateArgumentLoc* ArgsAsWritten = nullptr;
  unsigned NumArgsAsWritten = 0;
  const clang::TemplateArgumentList* Args = nullptr;
  if (FD->getDescribedFunctionTemplate() != nullptr) {
    // This is the body of a function template.
    return absl::nullopt;
  }
  if (auto* MSI = FD->getMemberSpecializationInfo()) {
    if (MSI->isExplicitSpecialization()) {
      // Refer to explicit specializations directly.
      return absl::nullopt;
    }
    // This is a member under a template instantiation. See #1879.
    return Observer.recordTappNode(
        BuildNodeIdForDecl(MSI->getInstantiatedFrom()), {}, 0);
  }
  std::vector<std::pair<clang::TemplateName, clang::SourceLocation>> TNs;
  if (auto* FTSI = FD->getTemplateSpecializationInfo()) {
    if (FTSI->isExplicitSpecialization()) {
      // Refer to explicit specializations directly.
      return absl::nullopt;
    }
    if (FTSI->TemplateArgumentsAsWritten) {
      // We have source locations for the template arguments.
      ArgsAsWritten = FTSI->TemplateArgumentsAsWritten->getTemplateArgs();
      NumArgsAsWritten = FTSI->TemplateArgumentsAsWritten->NumTemplateArgs;
    }
    Args = FTSI->TemplateArguments;
    TNs.emplace_back(TemplateName(FTSI->getTemplate()),
                     FTSI->getPointOfInstantiation());
  } else if (auto* DFTSI = FD->getDependentSpecializationInfo()) {
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
      for (unsigned I = 0; I < NumArgsAsWritten; ++I) {
        if (auto ArgId = BuildNodeIdForTemplateArgument(ArgsAsWritten[I])) {
          NIDS.push_back(ArgId.value());
        } else {
          CouldGetAllTypes = false;
          break;
        }
      }
    } else {
      NIDS.reserve(Args->size());
      for (unsigned I = 0; I < Args->size(); ++I) {
        if (auto ArgId = BuildNodeIdForTemplateArgument(Args->get(I))) {
          NIDS.push_back(ArgId.value());
        } else {
          CouldGetAllTypes = false;
          break;
        }
      }
    }
    if (CouldGetAllTypes) {
      // If there's more than one possible template name (e.g., this is
      // dependent), choose one arbitrarily.
      for (const auto& TN : TNs) {
        if (auto SpecializedNode = BuildNodeIdForTemplateName(TN.first)) {
          return Observer.recordTappNode(SpecializedNode.value(), NIDS,
                                         NumArgsAsWritten);
        }
      }
    }
  }
  return absl::nullopt;
}

GraphObserver::NodeId IndexerASTVisitor::BuildNodeIdForRefToDecl(
    const clang::Decl* Decl) {
  if (auto TappId = BuildNodeIdForImplicitTemplateInstantiation(Decl)) {
    return TappId.value();
  }
  return BuildNodeIdForDecl(Decl);
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForRefToDeclContext(
    const clang::DeclContext* DC) {
  if (auto* DCDecl = llvm::dyn_cast<const clang::Decl>(DC)) {
    if (auto TappId = BuildNodeIdForImplicitTemplateInstantiation(DCDecl)) {
      return TappId;
    }
    return BuildNodeIdForDeclContext(DC);
  }
  return absl::nullopt;
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForDeclContext(const clang::DeclContext* DC) {
  if (auto* DCDecl = llvm::dyn_cast<const clang::Decl>(DC)) {
    if (llvm::isa<TranslationUnitDecl>(DCDecl)) {
      return absl::nullopt;
    }
    if (llvm::isa<ClassTemplatePartialSpecializationDecl>(DCDecl)) {
      return BuildNodeIdForDecl(DCDecl, 0);
    } else if (auto* CRD = dyn_cast<const clang::CXXRecordDecl>(DCDecl)) {
      if (const auto* CTD = CRD->getDescribedClassTemplate()) {
        return BuildNodeIdForDecl(DCDecl, 0);
      }
    } else if (auto* FD = dyn_cast<const clang::FunctionDecl>(DCDecl)) {
      if (FD->getDescribedFunctionTemplate()) {
        return BuildNodeIdForDecl(DCDecl, 0);
      }
    }
    return BuildNodeIdForDecl(DCDecl);
  }
  return absl::nullopt;
}

void IndexerASTVisitor::AddChildOfEdgeToDeclContext(
    const clang::Decl* Decl, const GraphObserver::NodeId& DeclNode) {
  if (const DeclContext* DC = Decl->getDeclContext()) {
    if (absl::GetFlag(FLAGS_experimental_alias_template_instantiations)) {
      if (!Job->UnderneathImplicitTemplateInstantiation) {
        if (auto ContextId = BuildNodeIdForRefToDeclContext(DC)) {
          Observer.recordChildOfEdge(DeclNode, ContextId.value());
        }
      }
    } else {
      if (auto ContextId = BuildNodeIdForDeclContext(DC)) {
        Observer.recordChildOfEdge(DeclNode, ContextId.value());
      }
    }
  }
}

bool IndexerASTVisitor::VisitSizeOfPackExpr(const clang::SizeOfPackExpr* Expr) {
  if (auto RCC = ExpandedRangeInCurrentContext(Expr->getPackLoc())) {
    auto NodeId = BuildNodeIdForRefToDecl(Expr->getPack());
    Observer.recordDeclUseLocation(
        *RCC, NodeId, GraphObserver::Claimability::Claimable, IsImplicit(*RCC));
  }
  return true;
}

bool IndexerASTVisitor::VisitDeclRefExpr(const clang::DeclRefExpr* DRE) {
  return VisitDeclRefOrIvarRefExpr(DRE, DRE->getDecl(), DRE->getLocation());
}

bool IndexerASTVisitor::VisitBuiltinTypeLoc(clang::BuiltinTypeLoc TL) {
  if (absl::GetFlag(FLAGS_emit_anchors_on_builtins)) {
    RecordTypeLocSpellingLocation(TL);
  }
  return true;
}

bool IndexerASTVisitor::VisitEnumTypeLoc(clang::EnumTypeLoc TL) {
  RecordTypeLocSpellingLocation(TL);
  return true;
}

bool IndexerASTVisitor::VisitRecordTypeLoc(clang::RecordTypeLoc TL) {
  // This is a hack to see if we're being visited as part of a construct expr
  // constructing the type in question. When visiting a CXXConstructExpr, we
  // emit the ref/id to the class in question directly.
  if (!Job->ConstructorStack.empty() &&
      Job->ConstructorStack.back()
              ->getType()
              ->getAsAdjusted<clang::RecordType>() == TL.getTypePtr()) {
    return true;
  }
  RecordTypeLocSpellingLocation(TL);
  return true;
}

bool IndexerASTVisitor::VisitTemplateTypeParmTypeLoc(
    clang::TemplateTypeParmTypeLoc TL) {
  RecordTypeLocSpellingLocation(TL);
  return true;
}

template <typename TypeLoc, typename Type>
bool IndexerASTVisitor::VisitTemplateSpecializationTypePairHelper(
    TypeLoc Written, const Type* Resolved) {
  auto NameLocation = Written.getTemplateNameLoc();
  if (NameLocation.isFileID()) {
    if (auto RCC = ExpandedRangeInCurrentContext(NameLocation)) {
      if (auto TemplateName =
              BuildNodeIdForTemplateName(Resolved->getTemplateName())) {
        NodeId DeclNode = [&] {
          if (const auto* Decl = Resolved->getAsCXXRecordDecl()) {
            return BuildNodeIdForDecl(Decl);
          }
          return TemplateName.value();
        }();
        Observer.recordDeclUseLocation(*RCC, DeclNode,
                                       GraphObserver::Claimability::Claimable,
                                       IsImplicit(*RCC));
      }
    }
  }
  RecordTypeLocSpellingLocation(Written, Resolved);
  return true;
}

bool IndexerASTVisitor::VisitTemplateSpecializationTypeLoc(
    clang::TemplateSpecializationTypeLoc TL) {
  return VisitTemplateSpecializationTypePairHelper(TL, TL.getTypePtr());
}

bool IndexerASTVisitor::VisitDeducedTemplateSpecializationTypePair(
    clang::DeducedTemplateSpecializationTypeLoc TL,
    const clang::DeducedTemplateSpecializationType* T) {
  return VisitTemplateSpecializationTypePairHelper(TL, T);
}

bool IndexerASTVisitor::VisitAutoTypePair(clang::AutoTypeLoc TL,
                                          const clang::AutoType* T) {
  RecordTypeLocSpellingLocation(TL, T);
  return true;
}

bool IndexerASTVisitor::VisitSubstTemplateTypeParmTypeLoc(
    clang::SubstTemplateTypeParmTypeLoc TL) {
  // TODO(zarko): Record both the replaced parameter and the replacement
  // type.
  RecordTypeLocSpellingLocation(TL);
  return true;
}

bool IndexerASTVisitor::VisitDecltypeTypeLoc(clang::DecltypeTypeLoc TL) {
  RecordTypeLocSpellingLocation(TL);
  return true;
}

bool IndexerASTVisitor::VisitElaboratedTypeLoc(clang::ElaboratedTypeLoc TL) {
  // This one wraps a qualified (via 'struct S' | 'N::M::type') type
  // reference and we don't want to link 'struct'.
  // TODO(zarko): Add an anchor for all the Elaborated type; otherwise decls
  // like `typedef B::C tdef;` will only anchor `C` instead of `B::C`.
  return true;
}

bool IndexerASTVisitor::VisitTypedefTypeLoc(clang::TypedefTypeLoc TL) {
  RecordTypeLocSpellingLocation(TL);
  return true;
}

bool IndexerASTVisitor::VisitInjectedClassNameTypeLoc(
    clang::InjectedClassNameTypeLoc TL) {
  RecordTypeLocSpellingLocation(TL);
  return true;
}

bool IndexerASTVisitor::VisitDependentNameTypeLoc(
    clang::DependentNameTypeLoc TL) {
  if (auto Nodes = RecordTypeLocSpellingLocation(TL)) {
    if (auto RCC =
            ExplicitRangeInCurrentContext(NormalizeRange(TL.getNameLoc()))) {
      Observer.recordDeclUseLocation(*RCC, Nodes.ForReference(),
                                     GraphObserver::Claimability::Claimable,
                                     IsImplicit(*RCC));
    }
    if (RecordParamEdgesForDependentName(Nodes.ForReference(),
                                         TL.getQualifierLoc())) {
      RecordLookupEdgeForDependentName(
          Nodes.ForReference(),
          clang::DeclarationName(TL.getTypePtr()->getIdentifier()));
    }
  }
  return true;
}

bool IndexerASTVisitor::VisitPackExpansionTypeLoc(
    clang::PackExpansionTypeLoc TL) {
  RecordTypeLocSpellingLocation(TL);
  return true;
}

bool IndexerASTVisitor::VisitObjCObjectTypeLoc(clang::ObjCObjectTypeLoc TL) {
  for (unsigned i = 0; i < TL.getNumProtocols(); ++i) {
    if (auto RCC = ExpandedRangeInCurrentContext(TL.getProtocolLoc(i))) {
      Observer.recordDeclUseLocation(*RCC,
                                     BuildNodeIdForDecl(TL.getProtocol(i)),
                                     Claimability::Claimable, IsImplicit(*RCC));
    }
  }

  RecordTypeLocSpellingLocation(TL);
  return true;
}

bool IndexerASTVisitor::VisitObjCTypeParamTypeLoc(
    clang::ObjCTypeParamTypeLoc TL) {
  RecordTypeLocSpellingLocation(TL);
  return true;
}

bool IndexerASTVisitor::TraverseAttributedTypeLoc(clang::AttributedTypeLoc TL) {
  // TODO(shahms): Emit a reference to the underlying TL covering the entire
  // range of attributed TL.  This was the desired behavior previously, but not
  // implemented as such.
  //
  // If we have an attributed type, treat it as the raw type, but provide the
  // source range of the attributed type. This allows us to connect a _Nonnull
  // type back to the underlying type's definition. For example:
  // `@property Data * _Nullable var;` should connect back to the type Data *,
  // not Data * _Nullable;
  return Base::TraverseAttributedTypeLoc(TL);
}

bool IndexerASTVisitor::TraverseDependentAddressSpaceTypeLoc(
    clang::DependentAddressSpaceTypeLoc TL) {
  // TODO(shahms): Emit a reference to the underlying TL covering the entire
  // range of attributed TL.  This was the desired behavior previously, but not
  // implemented as such.
  //
  // If we have an attributed type, treat it as the raw type, but provide the
  // source range of the attributed type. This allows us to connect a _Nonnull
  // type back to the underlying type's definition. For example:
  // `@property Data * _Nullable var;` should connect back to the type Data *,
  // not Data * _Nullable;
  return Base::TraverseDependentAddressSpaceTypeLoc(TL);
}

bool IndexerASTVisitor::TraverseMemberPointerTypeLoc(
    clang::MemberPointerTypeLoc TL) {
  // TODO(shahms): Fix this upstream. RecursiveASTVisitor calls:
  //   TraverseType(Class);
  //   TraverseTypeLoc(Pointee);
  // Rather than TraverseTypeLoc on both.
  if (auto* TSI = TL.getClassTInfo()) {
    return TraverseTypeLoc(TSI->getTypeLoc()) &&
           TraverseTypeLoc(TL.getPointeeLoc());
  } else {
    return Base::TraverseMemberPointerTypeLoc(TL);
  }
}

bool IndexerASTVisitor::VisitDesignatedInitExpr(
    const clang::DesignatedInitExpr* DIE) {
  for (const auto& D : DIE->designators()) {
    if (!D.isFieldDesignator()) continue;
    if (const auto* F = D.getField()) {
      if (!VisitDeclRefOrIvarRefExpr(DIE, F, D.getFieldLoc(), false)) {
        return false;
      }
    }
  }
  return true;
}

bool IndexerASTVisitor::VisitInitListExpr(const clang::InitListExpr* ILE) {
  // We need the resolved type of the InitListExpr and all of the fields, but
  // don't want to do so redundantly for both syntactic and semantic forms.
  // Skip emitting ref/init if there aren't any syntactic initializers.
  const auto* SynILE = GetSyntacticForm(ILE);
  if (!ILE->isSemanticForm() || SynILE->getNumInits() == 0) {
    return true;
  }

  // SourceRange covering the *syntactic* initializers.
  clang::SourceRange ListRange =
      NormalizeRange({(*SynILE->inits().begin())->getBeginLoc(),
                      (*SynILE->inits().rbegin())->getEndLoc()});
  if (!ListRange.isValid()) {
    return true;
  }

  auto II = ILE->inits().begin();
  for (const clang::Decl* Decl : GetInitExprDecls(ILE)) {
    if (II == ILE->inits().end()) {
      LogErrorWithASTDump("Fewer initializers than decls:\n", ILE);
      break;
    }
    // On rare occasions, the init Expr we get from clang is null.
    if (const clang::Expr* Init = *II++) {
      clang::SourceRange InitRange = NormalizeRange(Init->getSourceRange());
      if (!(InitRange.isValid() && ListRange.fullyContains(InitRange))) {
        // When visiting the semantic form initializers which aren't explicitly
        // specified either have an invalid location (for uninitialized fields)
        // or share a location with the end of the ILE (for default initialized
        // fields). Skip these by checking that their location is wholly
        // contained by the syntactic initializers, rather than the enclosing
        // ILE itself.
        continue;
      }
      if (auto RCC = RangeInCurrentContext(BuildNodeIdForImplicitStmt(Init),
                                           InitRange)) {
        Observer.recordInitLocation(*RCC, BuildNodeIdForRefToDecl(Decl),
                                    GraphObserver::Claimability::Unclaimable,
                                    this->IsImplicit(*RCC));
      }
    }
  }
  return true;
}

bool IndexerASTVisitor::TraverseBinaryOperator(clang::BinaryOperator* BO) {
  if (BO->getOpcode() != clang::BO_Assign)
    return Base::TraverseBinaryOperator(BO);

  if (auto rhs = BO->getRHS(), lhs = BO->getLHS();
      lhs != nullptr && rhs != nullptr) {
    if (!WalkUpFromBinaryOperator(BO)) return false;
    if (!TraverseStmt(lhs)) return false;
    auto scope_guard = PushScope(Job->InfluenceSets, {});
    if (!TraverseStmt(rhs)) {
      return false;
    }
    if (auto expr = llvm::dyn_cast_or_null<clang::DeclRefExpr>(lhs);
        expr != nullptr && expr->getFoundDecl() != nullptr &&
        expr->getFoundDecl()->getKind() == clang::Decl::Kind::Var) {
      for (const auto* decl : Job->InfluenceSets.back()) {
        Observer.recordInfluences(BuildNodeIdForDecl(decl),
                                  BuildNodeIdForDecl(expr->getFoundDecl()));
      }
    }
    return true;
  }
  return Base::TraverseBinaryOperator(BO);
}

bool IndexerASTVisitor::TraverseInitListExpr(clang::InitListExpr* ILE) {
  if (ILE == nullptr) return true;
  // Because we visit implicit code, the default traversal will visit both
  // syntactic and semantic forms, resulting in duplicate edges.  This is
  // because only the syntactic form retains designated initializers while the
  // semantic form is required for mapping from initializing expression to the
  // field/base which it initializes.
  if (ILE->isSemanticForm() && ILE->isSyntacticForm()) {
    return Base::TraverseSynOrSemInitListExpr(ILE);
  }

  // The only thing we need from the syntactic form are designated initializers,
  // so visit them directly, but don't recurse into InitListExprs.
  struct DesignatedInitVisitor : RecursiveASTVisitor<DesignatedInitVisitor> {
    bool VisitDesignatedInitExpr(const clang::DesignatedInitExpr* DIE) {
      return parent.VisitDesignatedInitExpr(DIE);
    }

    bool TraverseInitListExpr(const clang::InitListExpr*) {
      return true;  // Disable recursion on InitListExpr.
    }

    IndexerASTVisitor& parent;
  } visitor{{}, *this};

  return visitor.TraverseSynOrSemInitListExpr(GetSyntacticForm(ILE)) &&
         Base::TraverseSynOrSemInitListExpr(GetSemanticForm(ILE));
}

NodeSet IndexerASTVisitor::RecordTypeLocSpellingLocation(clang::TypeLoc TL) {
  return RecordTypeLocSpellingLocation(TL, TL.getTypePtr());
}

NodeSet IndexerASTVisitor::RecordTypeLocSpellingLocation(
    clang::TypeLoc Written, const clang::Type* Resolved) {
  if (auto RCC = ExpandedRangeInCurrentContext(Written.getSourceRange())) {
    if (auto Nodes = BuildNodeSetForType(Resolved)) {
      Observer.recordTypeSpellingLocation(
          *RCC, Nodes.ForReference(), Nodes.claimability(), IsImplicit(*RCC));
      return Nodes;
    }
  }
  return NodeSet::Empty();
}

bool IndexerASTVisitor::TraverseDeclarationNameInfo(
    clang::DeclarationNameInfo NameInfo) {
  switch (NameInfo.getName().getNameKind()) {
    case DeclarationName::CXXConversionFunctionName:
      // For ConversionFunctions this leads to duplicate edges as the return
      // value is visited both here and via TraverseFunctionProtoTypeLoc.
      return true;
    case DeclarationName::CXXConstructorName:
    case DeclarationName::CXXDestructorName:
      if (clang::TypeSourceInfo* TSI = NameInfo.getNamedTypeInfo()) {
        if (auto RCC = ExpandedRangeInCurrentContext(
                TSI->getTypeLoc().getSourceRange())) {
          if (auto Nodes =
                  BuildNodeSetForType(TSI->getTypeLoc().getTypePtr())) {
            Observer.recordTypeIdSpellingLocation(*RCC, Nodes.ForReference(),
                                                  Nodes.claimability(),
                                                  IsImplicit(*RCC));
          }
        }
        return true;
      }
    default:
      break;
  }
  return Base::TraverseDeclarationNameInfo(NameInfo);
}

// Use FoundDecl to get to template defs; use getDecl to get to template
// instantiations.
bool IndexerASTVisitor::VisitDeclRefOrIvarRefExpr(
    const clang::Expr* Expr, const NamedDecl* const FoundDecl,
    SourceLocation SL, bool IsImplicit) {
  // TODO(zarko): check to see if this DeclRefExpr has already been indexed.
  // (Use a simple N=1 cache.)
  // TODO(zarko): Point at the capture as well as the thing being captured;
  // port over RemapDeclIfCaptured.
  // const NamedDecl* const TargetDecl = RemapDeclIfCaptured(FoundDecl);
  if (isa<clang::VarDecl>(FoundDecl) && FoundDecl->isImplicit()) {
    // Ignore variable declarations synthesized from for-range loops, as they
    // are just a clang implementation detail.
    return true;
  }
  if (SL.isValid()) {
    SourceRange Range = NormalizeRange(SL);
    if (IsImplicit) {
      // Mark implicit ranges by making them zero-length.
      Range.setEnd(Range.getBegin());
    }

    auto StmtId = BuildNodeIdForImplicitStmt(Expr);
    if (auto RCC = RangeInCurrentContext(StmtId, Range)) {
      GraphObserver::NodeId DeclId = BuildNodeIdForRefToDecl(FoundDecl);
      if (ShouldHaveBlameContext(FoundDecl)) {
        if (Job->BlameStack.empty()) {
          if (auto FileId = Observer.recordFileInitializer(*RCC)) {
            Observer.recordBlameLocation(
                *RCC, *FileId, GraphObserver::Claimability::Unclaimable,
                this->IsImplicit(*RCC));
          }
        } else {
          for (const auto& Context : Job->BlameStack.back()) {
            Observer.recordBlameLocation(
                *RCC, Context, GraphObserver::Claimability::Unclaimable,
                this->IsImplicit(*RCC));
          }
        }
      }
      auto semantic = IsUsedAsWrite(*getAllParents(), Expr)
                          ? GraphObserver::UseKind::kWrite
                          : GraphObserver::UseKind::kUnknown;
      if (!Job->InfluenceSets.empty() &&
          FoundDecl->getKind() == clang::Decl::Kind::Var) {
        Job->InfluenceSets.back().insert(FoundDecl);
      }
      Observer.recordSemanticDeclUseLocation(
          *RCC, DeclId, semantic, GraphObserver::Claimability::Unclaimable,
          this->IsImplicit(*RCC));
      for (const auto& S : Supports) {
        S->InspectDeclRef(*this, SL, *RCC, DeclId, FoundDecl);
      }
    }
  }
  return true;
}

absl::optional<std::vector<GraphObserver::NodeId>>
IndexerASTVisitor::BuildTemplateArgumentList(ArrayRef<TemplateArgument> Args) {
  std::vector<NodeId> result;
  result.reserve(Args.size());
  for (const auto& Arg : Args) {
    if (auto ArgId = BuildNodeIdForTemplateArgument(Arg)) {
      result.push_back(*ArgId);
    } else {
      return absl::nullopt;
    }
  }
  return result;
}

absl::optional<std::vector<GraphObserver::NodeId>>
IndexerASTVisitor::BuildTemplateArgumentList(
    ArrayRef<TemplateArgumentLoc> Args) {
  std::vector<NodeId> result;
  result.reserve(Args.size());
  for (const auto& ArgLoc : Args) {
    if (auto ArgId = BuildNodeIdForTemplateArgument(ArgLoc)) {
      result.push_back(*ArgId);
    } else {
      return absl::nullopt;
    }
  }
  return result;
}

bool IndexerASTVisitor::VisitVarDecl(const clang::VarDecl* Decl) {
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
  const clang::ASTTemplateArgumentListInfo* ArgsAsWritten = nullptr;
  if (const auto* VTPSD =
          dyn_cast<const clang::VarTemplatePartialSpecializationDecl>(Decl)) {
    ArgsAsWritten = VTPSD->getTemplateArgsAsWritten();
    BodyDeclNode = BuildNodeIdForDecl(Decl, 0);
    DeclNode = RecordTemplate(VTPSD, BodyDeclNode);
  } else if (const auto* VTD = Decl->getDescribedVarTemplate()) {
    CHECK(!isa<clang::VarTemplateSpecializationDecl>(VTD));
    BodyDeclNode = BuildNodeIdForDecl(Decl);
    DeclNode = RecordTemplate(VTD, BodyDeclNode);
  } else {
    BodyDeclNode = BuildNodeIdForDecl(Decl);
    DeclNode = BodyDeclNode;
  }

  // if this variable is a static member record it as static
  if (Decl->isStaticDataMember()) {
    Observer.recordStaticVariable(DeclNode);
  }

  if (auto* VTSD = dyn_cast<const clang::VarTemplateSpecializationDecl>(Decl)) {
    Marks.set_implicit(Job->UnderneathImplicitTemplateInstantiation ||
                       !VTSD->isExplicitInstantiationOrSpecialization());
    // If this is a partial specialization, we've already recorded the newly
    // abstracted parameters above. We can now record the type arguments passed
    // to the template we're specializing. Synthesize the type we need.
    auto PrimaryOrPartial = VTSD->getSpecializedTemplateOrPartial();
    if (auto NIDS = (ArgsAsWritten
                         ? BuildTemplateArgumentList(ArgsAsWritten->arguments())
                         : BuildTemplateArgumentList(
                               VTSD->getTemplateArgs().asArray()))) {
      if (auto SpecializedNode = BuildNodeIdForTemplateName(
              clang::TemplateName(VTSD->getSpecializedTemplate()))) {
        if (PrimaryOrPartial.is<clang::VarTemplateDecl*>() &&
            !isa<const clang::VarTemplatePartialSpecializationDecl>(Decl)) {
          // This is both an instance and a specialization of the primary
          // template. We use the same arguments list for both.
          Observer.recordInstEdge(
              DeclNode, Observer.recordTappNode(SpecializedNode.value(), *NIDS),
              GraphObserver::Confidence::NonSpeculative);
        }
        Observer.recordSpecEdge(
            DeclNode, Observer.recordTappNode(SpecializedNode.value(), *NIDS),
            GraphObserver::Confidence::NonSpeculative);
      }
    }
    if (const auto* Partial =
            PrimaryOrPartial
                .dyn_cast<clang::VarTemplatePartialSpecializationDecl*>()) {
      if (auto NIDS = BuildTemplateArgumentList(
              VTSD->getTemplateInstantiationArgs().asArray())) {
        Observer.recordInstEdge(
            DeclNode,
            Observer.recordTappNode(BuildNodeIdForDecl(Partial), *NIDS),
            GraphObserver::Confidence::NonSpeculative);
      }
    }
  }

  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode,
      BuildNodeIdForDefnOfDecl(Decl));
  Marks.set_marked_source_end(NormalizeRange(Decl->getSourceRange()).getEnd());
  Marks.set_name_range(NameRange);
  if (auto TyNodeId = BuildNodeIdForType(Decl->getType())) {
    Observer.recordTypeEdge(BodyDeclNode, *TyNodeId);
  }
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  std::vector<LibrarySupport::Completion> Completions;
  if (!IsDefinition(Decl)) {
    AssignUSR(BodyDeclNode, Decl);
    Observer.recordVariableNode(
        BodyDeclNode, GraphObserver::Completeness::Incomplete,
        GraphObserver::VariableSubkind::None, absl::nullopt);
    Observer.recordMarkedSource(DeclNode, Marks.GenerateMarkedSource(DeclNode));
    for (const auto& S : Supports) {
      S->InspectVariable(*this, DeclNode, BodyDeclNode, Decl,
                         GraphObserver::Completeness::Incomplete, Completions);
    }
    return true;
  }
  FileID DeclFile = Observer.getSourceManager()->getFileID(Decl->getLocation());
  auto NameRangeInContext =
      RangeInCurrentContext(Decl->isImplicit(), BodyDeclNode, NameRange);
  for (const auto* NextDecl : Decl->redecls()) {
    const clang::Decl* OuterTemplate = nullptr;
    // It's not useful to draw completion edges to implicit forward
    // declarations, nor is it useful to declare that a definition completes
    // itself.
    if (NextDecl != Decl && !NextDecl->isImplicit()) {
      if (auto* VD = dyn_cast<const clang::VarDecl>(NextDecl)) {
        OuterTemplate = VD->getDescribedVarTemplate();
      }
      FileID NextDeclFile =
          Observer.getSourceManager()->getFileID(NextDecl->getLocation());
      // We should not point a completes edge from an abs node to a var node.
      GraphObserver::NodeId TargetDecl =
          BuildNodeIdForDecl(OuterTemplate ? OuterTemplate : NextDecl);
      if (NameRangeInContext) {
        Observer.recordCompletionRange(
            NameRangeInContext.value(), TargetDecl,
            NextDeclFile == DeclFile
                ? GraphObserver::Specificity::UniquelyCompletes
                : GraphObserver::Specificity::Completes,
            DeclNode);
      }
      Completions.push_back(LibrarySupport::Completion{NextDecl, TargetDecl});
    }
  }
  AssignUSR(BodyDeclNode, Decl);
  Observer.recordVariableNode(
      BodyDeclNode, GraphObserver::Completeness::Definition,
      GraphObserver::VariableSubkind::None, absl::nullopt);
  Observer.recordMarkedSource(DeclNode, Marks.GenerateMarkedSource(DeclNode));
  for (const auto& S : Supports) {
    S->InspectVariable(*this, DeclNode, BodyDeclNode, Decl,
                       GraphObserver::Completeness::Definition, Completions);
  }
  return true;
}

bool IndexerASTVisitor::VisitNamespaceDecl(const clang::NamespaceDecl* Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  // Use the range covering `namespace` for anonymous namespaces.
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  // Namespaces are never defined; they are only invoked.
  if (auto RCC =
          RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange)) {
    Observer.recordDeclUseLocation(RCC.value(), DeclNode,
                                   GraphObserver::Claimability::Unclaimable,
                                   IsImplicit(RCC.value()));
  }
  Observer.recordNamespaceNode(DeclNode, Marks.GenerateMarkedSource(DeclNode));
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  return true;
}

bool IndexerASTVisitor::VisitBindingDecl(const clang::BindingDecl* Decl) {
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  GraphObserver::NodeId DeclNode = BuildNodeIdForDecl(Decl);

  if (auto RCC = ExplicitRangeInCurrentContext(NameRange)) {
    Observer.recordDefinitionBindingRange(RCC.value(), DeclNode, absl::nullopt);
  }

  auto Marks = MarkedSources.Generate(Decl);
  Marks.set_marked_source_end(Decl->getSourceRange().getEnd());

  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  Observer.recordVariableNode(DeclNode, GraphObserver::Completeness::Definition,
                              GraphObserver::VariableSubkind::None,
                              Marks.GenerateMarkedSource(DeclNode));
  return true;
}

bool IndexerASTVisitor::VisitFieldDecl(const clang::FieldDecl* Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode,
      absl::nullopt);
  Marks.set_implicit(Job->UnderneathImplicitTemplateInstantiation);
  Marks.set_marked_source_end(NormalizeRange(Decl->getSourceRange()).getEnd());
  Marks.set_name_range(NameRange);
  // TODO(zarko): Record completeness data. This is relevant for static fields,
  // which may be declared along with a complete class definition but later
  // defined in a separate translation unit.
  Observer.recordVariableNode(DeclNode, GraphObserver::Completeness::Definition,
                              GraphObserver::VariableSubkind::Field,
                              Marks.GenerateMarkedSource(DeclNode));
  AssignUSR(DeclNode, Decl);
  if (auto TyNodeId = BuildNodeIdForType(Decl->getType())) {
    Observer.recordTypeEdge(DeclNode, *TyNodeId);
  }
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  return true;
}

bool IndexerASTVisitor::VisitEnumConstantDecl(
    const clang::EnumConstantDecl* Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  Marks.set_implicit(Job->UnderneathImplicitTemplateInstantiation);
  // We first build the NameId and NodeId for the enumerator.
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  SourceLocation DeclLoc = Decl->getLocation();
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  Marks.set_marked_source_end(NormalizeRange(Decl->getSourceRange()).getEnd());
  Marks.set_name_range(NameRange);
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode,
      absl::nullopt);
  AssignUSR(DeclNode, Decl);
  Observer.recordIntegerConstantNode(DeclNode, Decl->getInitVal());
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  Observer.recordMarkedSource(DeclNode, Marks.GenerateMarkedSource(DeclNode));
  return true;
}

bool IndexerASTVisitor::VisitEnumDecl(const clang::EnumDecl* Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  Marks.set_implicit(Job->UnderneathImplicitTemplateInstantiation);
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  SourceLocation DeclLoc = Decl->getLocation();
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  if (Decl->isThisDeclarationADefinition() && Decl->getBody() != nullptr) {
    Marks.set_marked_source_end(Decl->getBody()->getSourceRange().getBegin());
  } else {
    Marks.set_marked_source_end(
        NormalizeRange(Decl->getSourceRange()).getEnd());
  }
  Marks.set_name_range(NameRange);
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode,
      BuildNodeIdForDefnOfDecl(Decl));
  bool HasSpecifiedStorageType = false;
  if (const auto* TSI = Decl->getIntegerTypeSourceInfo()) {
    HasSpecifiedStorageType = true;
    if (auto TyNodeId = BuildNodeIdForType(TSI->getType())) {
      Observer.recordTypeEdge(DeclNode, *TyNodeId);
    }
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
    AssignUSR(DeclNode, Decl);
    Observer.recordEnumNode(
        DeclNode,
        HasSpecifiedStorageType ? GraphObserver::Completeness::Complete
                                : GraphObserver::Completeness::Incomplete,
        Decl->isScoped() ? GraphObserver::EnumKind::Scoped
                         : GraphObserver::EnumKind::Unscoped);
    return true;
  }
  FileID DeclFile = Observer.getSourceManager()->getFileID(Decl->getLocation());
  for (const auto* NextDecl : Decl->redecls()) {
    if (NextDecl != Decl) {
      FileID NextDeclFile =
          Observer.getSourceManager()->getFileID(NextDecl->getLocation());
      if (auto RCC =
              RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange)) {
        Observer.recordCompletionRange(
            RCC.value(), BuildNodeIdForDecl(NextDecl),
            NextDeclFile == DeclFile
                ? GraphObserver::Specificity::UniquelyCompletes
                : GraphObserver::Specificity::Completes,
            DeclNode);
      }
    }
  }
  AssignUSR(DeclNode, Decl);
  Observer.recordEnumNode(DeclNode, GraphObserver::Completeness::Definition,
                          Decl->isScoped() ? GraphObserver::EnumKind::Scoped
                                           : GraphObserver::EnumKind::Unscoped);
  return true;
}

// TODO(zarko): In general, while we traverse a specialization we don't
// want to have the primary-template's type variables in context.
bool IndexerASTVisitor::TraverseClassTemplateDecl(
    clang::ClassTemplateDecl* TD) {
  auto Scope = PushScope(Job->TypeContext, TD->getTemplateParameters());
  return Base::TraverseClassTemplateDecl(TD);
}

// NB: The Traverse* member that's called is based on the dynamic type of the
// AST node it's being called with (so only one of
// TraverseClassTemplate{Partial}SpecializationDecl will be called).
bool IndexerASTVisitor::TraverseClassTemplateSpecializationDecl(
    clang::ClassTemplateSpecializationDecl* TD) {
  auto Scope = std::make_tuple(
      RestoreStack(Job->RangeContext),
      RestoreValue(Job->UnderneathImplicitTemplateInstantiation));
  // If this specialization was spelled out in the file, it has
  // physical ranges.
  if (TD->getTemplateSpecializationKind() !=
      clang::TSK_ExplicitSpecialization) {
    Job->RangeContext.push_back(BuildNodeIdForDecl(TD));
  }
  if (!TD->isExplicitInstantiationOrSpecialization()) {
    Job->UnderneathImplicitTemplateInstantiation = true;
  }
  return Base::TraverseClassTemplateSpecializationDecl(TD);
}

bool IndexerASTVisitor::TraverseVarTemplateSpecializationDecl(
    clang::VarTemplateSpecializationDecl* TD) {
  if (TD->getTemplateSpecializationKind() == TSK_Undeclared ||
      TD->getTemplateSpecializationKind() == TSK_ImplicitInstantiation) {
    // We should have hit implicit decls in the primary template.
    return true;
  } else {
    return ForceTraverseVarTemplateSpecializationDecl(TD);
  }
}

bool IndexerASTVisitor::ForceTraverseVarTemplateSpecializationDecl(
    clang::VarTemplateSpecializationDecl* TD) {
  auto Scope = std::make_tuple(
      RestoreStack(Job->RangeContext),
      RestoreValue(Job->UnderneathImplicitTemplateInstantiation));
  // If this specialization was spelled out in the file, it has
  // physical ranges.
  if (TD->getTemplateSpecializationKind() !=
      clang::TSK_ExplicitSpecialization) {
    Job->RangeContext.push_back(BuildNodeIdForDecl(TD));
  }
  if (!TD->isExplicitInstantiationOrSpecialization()) {
    Job->UnderneathImplicitTemplateInstantiation = true;
  }
  return Base::TraverseVarTemplateSpecializationDecl(TD);
}

bool IndexerASTVisitor::TraverseClassTemplatePartialSpecializationDecl(
    clang::ClassTemplatePartialSpecializationDecl* TD) {
  // Implicit partial specializations don't happen, so we don't need
  // to consider changing the Job->RangeContext stack.
  auto Scope = PushScope(Job->TypeContext, TD->getTemplateParameters());
  return Base::TraverseClassTemplatePartialSpecializationDecl(TD);
}

bool IndexerASTVisitor::TraverseVarTemplateDecl(clang::VarTemplateDecl* TD) {
  auto Scope = PushScope(Job->TypeContext, TD->getTemplateParameters());
  if (!TraverseDecl(TD->getTemplatedDecl())) {
    return false;
  }
  if (TD == TD->getCanonicalDecl()) {
    for (auto* SD : TD->specializations()) {
      for (auto* RD : SD->redecls()) {
        auto* VD = cast<VarTemplateSpecializationDecl>(RD);
        switch (VD->getSpecializationKind()) {
          // Visit the implicit instantiations with the requested pattern.
          case TSK_Undeclared:
          case TSK_ImplicitInstantiation:
            if (!ForceTraverseVarTemplateSpecializationDecl(VD)) {
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
  return true;
}

bool IndexerASTVisitor::TraverseVarTemplatePartialSpecializationDecl(
    clang::VarTemplatePartialSpecializationDecl* TD) {
  // Implicit partial specializations don't happen, so we don't need
  // to consider changing the Job->RangeContext stack.
  auto Scope = PushScope(Job->TypeContext, TD->getTemplateParameters());
  return Base::TraverseVarTemplatePartialSpecializationDecl(TD);
}

bool IndexerASTVisitor::TraverseTypeAliasTemplateDecl(
    clang::TypeAliasTemplateDecl* TATD) {
  auto Scope = PushScope(Job->TypeContext, TATD->getTemplateParameters());
  return TraverseDecl(TATD->getTemplatedDecl());
}

bool IndexerASTVisitor::TraverseFunctionDecl(clang::FunctionDecl* FD) {
  auto UITI = RestoreValue(Job->UnderneathImplicitTemplateInstantiation);
  if (const auto* MSI = FD->getMemberSpecializationInfo()) {
    // The definitions of class template member functions are not necessarily
    // dominated by the class template definition.
    if (!MSI->isExplicitSpecialization()) {
      Job->UnderneathImplicitTemplateInstantiation = true;
    }
  } else if (const auto* FSI = FD->getTemplateSpecializationInfo()) {
    if (!FSI->isExplicitInstantiationOrSpecialization()) {
      Job->UnderneathImplicitTemplateInstantiation = true;
    }
  }
  return Base::TraverseFunctionDecl(FD) && [&] {
    // RecursiveASTVisitor, mistakenly, does not visit these, likely
    // due to the obscure case in which they occur:
    // From Clang's documentation:
    // "Since explicit function template specialization and instantiation
    // declarations can only appear in namespace scope, and you can only
    // specialize a member of a fully-specialized class, the only way to get
    // one of these is in a friend declaration like the following:
    //   template <typename T> void f(T t);
    //   template <typename T> struct S {
    //     friend void f<>(T t);
    //   };
    // TODO(shahms): Fix this upstream by getting TraverseFunctionHelper to
    // do the right thing.
    if (auto* DFTSI = FD->getDependentSpecializationInfo()) {
      auto Count = DFTSI->getNumTemplateArgs();
      for (int i = 0; i < Count; i++) {
        if (!TraverseTemplateArgumentLoc(DFTSI->getTemplateArg(i))) {
          return false;
        }
      }
    }
    return true;
  }();
}

bool IndexerASTVisitor::TraverseFunctionTemplateDecl(
    clang::FunctionTemplateDecl* FTD) {
  {
    auto Scope = PushScope(Job->TypeContext, FTD->getTemplateParameters());
    // We traverse the template parameter list when we visit the FunctionDecl.
    TraverseDecl(FTD->getTemplatedDecl());
  }
  // See also RecursiveAstVisitor<T>::TraverseTemplateInstantiations.
  if (FTD == FTD->getCanonicalDecl()) {
    for (auto* FD : FTD->specializations()) {
      for (auto* RD : FD->redecls()) {
        if (RD->getTemplateSpecializationKind() !=
            clang::TSK_ExplicitSpecialization) {
          TraverseDecl(RD);
        }
      }
    }
  }
  return true;
}

absl::optional<GraphObserver::Range>
IndexerASTVisitor::ExplicitRangeInCurrentContext(const clang::SourceRange& SR) {
  if (!SR.getBegin().isValid()) {
    return absl::nullopt;
  }
  if (!Job->RangeContext.empty() &&
      !absl::GetFlag(FLAGS_experimental_alias_template_instantiations)) {
    return GraphObserver::Range(SR, Job->RangeContext.back());
  } else {
    return GraphObserver::Range(SR, Observer.getClaimTokenForRange(SR));
  }
}

absl::optional<GraphObserver::Range>
IndexerASTVisitor::ExpandedRangeInCurrentContext(clang::SourceRange SR) {
  if (!SR.isValid()) {
    return absl::nullopt;
  }
  return ExplicitRangeInCurrentContext(NormalizeRange(SR));
}

clang::SourceRange IndexerASTVisitor::NormalizeRange(
    clang::SourceRange SR) const {
  return ClangRangeFinder(Observer.getSourceManager(),
                          Observer.getLangOptions())
      .NormalizeRange(SR);
}

absl::optional<GraphObserver::Range> IndexerASTVisitor::RangeInCurrentContext(
    const absl::optional<GraphObserver::NodeId>& Id,
    const clang::SourceRange& SR) {
  if (Id) {
    return GraphObserver::Range::Implicit(Id.value(), SR);
  }
  return ExplicitRangeInCurrentContext(SR);
}

absl::optional<GraphObserver::Range> IndexerASTVisitor::RangeInCurrentContext(
    bool implicit, const GraphObserver::NodeId& Id,
    const clang::SourceRange& SR) {
  return implicit ? GraphObserver::Range(Id)
                  : ExplicitRangeInCurrentContext(SR);
}

GraphObserver::NodeId IndexerASTVisitor::RecordGenericClass(
    const ObjCInterfaceDecl* IDecl, const ObjCTypeParamList* TPL,
    const GraphObserver::NodeId& BodyId) {
  auto AbsId = BuildNodeIdForDecl(IDecl);
  Observer.recordAbsNode(AbsId);
  Observer.recordChildOfEdge(BodyId, AbsId);

  for (const ObjCTypeParamDecl* TP : *TPL) {
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
    const TemplateDeclish* Decl, const GraphObserver::NodeId& BodyDeclNode) {
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  Observer.recordChildOfEdge(BodyDeclNode, DeclNode);
  Observer.recordAbsNode(DeclNode);
  for (const auto* ND : *Decl->getTemplateParameters()) {
    GraphObserver::NodeId ParamId(Observer.getDefaultClaimToken(), "");
    unsigned ParamIndex = 0;
    auto Marks = MarkedSources.Generate(ND);
    if (const auto* TTPD = dyn_cast<clang::TemplateTypeParmDecl>(ND)) {
      ParamId = BuildNodeIdForDecl(ND);
      Observer.recordAbsVarNode(ParamId, Marks.GenerateMarkedSource(ParamId));
      ParamIndex = TTPD->getIndex();
    } else if (const auto* NTTPD =
                   dyn_cast<clang::NonTypeTemplateParmDecl>(ND)) {
      ParamId = BuildNodeIdForDecl(ND);
      Observer.recordAbsVarNode(ParamId, Marks.GenerateMarkedSource(ParamId));
      ParamIndex = NTTPD->getIndex();
    } else if (const auto* TTPD =
                   dyn_cast<clang::TemplateTemplateParmDecl>(ND)) {
      // We make the external Abs the primary node for TTPD so that
      // uses of the ParmDecl later on point at the Abs and not the wrapped
      // AbsVar.
      GraphObserver::NodeId ParamBodyId = BuildNodeIdForDecl(ND, 0);
      Observer.recordAbsVarNode(ParamBodyId,
                                Marks.GenerateMarkedSource(ParamBodyId));
      ParamId = RecordTemplate(TTPD, ParamBodyId);
      ParamIndex = TTPD->getIndex();
    } else {
      LOG(FATAL) << "Unknown entry in TemplateParameterList";
    }
    SourceRange Range = RangeForNameOfDeclaration(ND);
    MaybeRecordDefinitionRange(
        RangeInCurrentContext(Decl->isImplicit(), ParamId, Range), ParamId,
        absl::nullopt);
    Observer.recordParamEdge(DeclNode, ParamIndex, ParamId);
  }
  return DeclNode;
}

bool IndexerASTVisitor::VisitRecordDecl(const clang::RecordDecl* Decl) {
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
  const clang::ASTTemplateArgumentListInfo* ArgsAsWritten = nullptr;
  if (const auto* CTPSD =
          dyn_cast<const clang::ClassTemplatePartialSpecializationDecl>(Decl)) {
    ArgsAsWritten = CTPSD->getTemplateArgsAsWritten();
    BodyDeclNode = BuildNodeIdForDecl(Decl, 0);
    DeclNode = RecordTemplate(CTPSD, BodyDeclNode);
  } else if (auto* CRD = dyn_cast<const clang::CXXRecordDecl>(Decl)) {
    if (const auto* CTD = CRD->getDescribedClassTemplate()) {
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

  if (auto* CTSD =
          dyn_cast<const clang::ClassTemplateSpecializationDecl>(Decl)) {
    Marks.set_implicit(Job->UnderneathImplicitTemplateInstantiation ||
                       !CTSD->isExplicitInstantiationOrSpecialization());
    // If this is a partial specialization, we've already recorded the newly
    // abstracted parameters above. We can now record the type arguments passed
    // to the template we're specializing. Synthesize the type we need.
    auto PrimaryOrPartial = CTSD->getSpecializedTemplateOrPartial();
    if (auto NIDS = (ArgsAsWritten
                         ? BuildTemplateArgumentList(ArgsAsWritten->arguments())
                         : BuildTemplateArgumentList(
                               CTSD->getTemplateArgs().asArray()))) {
      if (auto SpecializedNode = BuildNodeIdForTemplateName(
              clang::TemplateName(CTSD->getSpecializedTemplate()))) {
        if (PrimaryOrPartial.is<clang::ClassTemplateDecl*>() &&
            !isa<const clang::ClassTemplatePartialSpecializationDecl>(Decl)) {
          // This is both an instance and a specialization of the primary
          // template. We use the same arguments list for both.
          Observer.recordInstEdge(
              DeclNode, Observer.recordTappNode(SpecializedNode.value(), *NIDS),
              GraphObserver::Confidence::NonSpeculative);
        }
        Observer.recordSpecEdge(
            DeclNode, Observer.recordTappNode(SpecializedNode.value(), *NIDS),
            GraphObserver::Confidence::NonSpeculative);
      }
    }
    if (const auto* Partial =
            PrimaryOrPartial
                .dyn_cast<clang::ClassTemplatePartialSpecializationDecl*>()) {
      if (auto NIDS = BuildTemplateArgumentList(
              CTSD->getTemplateInstantiationArgs().asArray())) {
        Observer.recordInstEdge(
            DeclNode,
            Observer.recordTappNode(BuildNodeIdForDecl(Partial), *NIDS),
            GraphObserver::Confidence::NonSpeculative);
      }
    }
  }
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode,
      BuildNodeIdForDefnOfDecl(Decl));
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
    AssignUSR(BodyDeclNode, Decl);
    Observer.recordRecordNode(BodyDeclNode, RK,
                              GraphObserver::Completeness::Incomplete,
                              absl::nullopt);
    Observer.recordMarkedSource(DeclNode, Marks.GenerateMarkedSource(DeclNode));
    return true;
  }
  FileID DeclFile = Observer.getSourceManager()->getFileID(Decl->getLocation());
  if (auto NameRangeInContext =
          RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange)) {
    for (const auto* NextDecl : Decl->redecls()) {
      const clang::Decl* OuterTemplate = nullptr;
      // It's not useful to draw completion edges to implicit forward
      // declarations, nor is it useful to declare that a definition completes
      // itself.
      if (NextDecl != Decl && !NextDecl->isImplicit()) {
        if (auto* CRD = dyn_cast<const clang::CXXRecordDecl>(NextDecl)) {
          OuterTemplate = CRD->getDescribedClassTemplate();
        }
        FileID NextDeclFile =
            Observer.getSourceManager()->getFileID(NextDecl->getLocation());
        // We should not point a completes edge from an abs node to a record
        // node.
        GraphObserver::NodeId TargetDecl =
            BuildNodeIdForDecl(OuterTemplate ? OuterTemplate : NextDecl);
        Observer.recordCompletionRange(
            NameRangeInContext.value(), TargetDecl,
            NextDeclFile == DeclFile
                ? GraphObserver::Specificity::UniquelyCompletes
                : GraphObserver::Specificity::Completes,
            DeclNode);
      }
    }
  }
  if (auto* CRD = dyn_cast<const clang::CXXRecordDecl>(Decl)) {
    for (const auto& BCS : CRD->bases()) {
      if (auto BCSType =
              BuildNodeIdForType(BCS.getTypeSourceInfo()->getTypeLoc())) {
        Observer.recordExtendsEdge(BodyDeclNode, BCSType.value(),
                                   BCS.isVirtual(), BCS.getAccessSpecifier());
      }
    }
  }
  AssignUSR(BodyDeclNode, Decl);
  Observer.recordRecordNode(
      BodyDeclNode, RK, GraphObserver::Completeness::Definition, absl::nullopt);
  Observer.recordMarkedSource(DeclNode, Marks.GenerateMarkedSource(DeclNode));
  return true;
}

bool IndexerASTVisitor::VisitFunctionDecl(clang::FunctionDecl* Decl) {
  if (SkipAliasedDecl(Decl)) {
    return true;
  }
  auto Marks = MarkedSources.Generate(Decl);
  GraphObserver::NodeId InnerNode(Observer.getDefaultClaimToken(), "");
  GraphObserver::NodeId OuterNode(Observer.getDefaultClaimToken(), "");
  // There are five flavors of function (see TemplateOrSpecialization in
  // FunctionDecl).
  const clang::TemplateArgumentLoc* ArgsAsWritten = nullptr;
  unsigned NumArgsAsWritten = 0;
  const clang::TemplateArgumentList* Args = nullptr;
  std::vector<std::pair<clang::TemplateName, clang::SourceLocation>> TNs;
  bool TNsAreSpeculative = false;
  bool IsImplicit = false;
  SourceLocation TemplateKeywordLoc;
  if (auto* FTD = Decl->getDescribedFunctionTemplate()) {
    // Function template (inc. overloads)
    InnerNode = BuildNodeIdForDecl(Decl, 0);
    OuterNode = RecordTemplate(FTD, InnerNode);
    TemplateKeywordLoc = FTD->getSourceRange().getBegin();
  } else if (auto* MSI = Decl->getMemberSpecializationInfo()) {
    // Here, template variables are bound by our enclosing context.
    // For example:
    // `template <typename T> struct S { int f() { return 0; } };`
    // `int q() { S<int> s; return s.f(); }`
    // We're going to be a little loose with `instantiates` here. To make
    // queries consistent, we'll use a nullary tapp() to refer to the
    // instantiated object underneath a template. It may be useful to also
    // add the type variables involve in this instantiation's particular
    // parent, but we don't currently do so. (#1879)
    IsImplicit = !MSI->isExplicitSpecialization();
    InnerNode = BuildNodeIdForDecl(Decl);
    OuterNode = InnerNode;
    Observer.recordInstEdge(
        OuterNode,
        Observer.recordTappNode(BuildNodeIdForDecl(MSI->getInstantiatedFrom()),
                                {}),
        GraphObserver::Confidence::NonSpeculative);
  } else if (auto* FTSI = Decl->getTemplateSpecializationInfo()) {
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
  } else if (auto* DFTSI = Decl->getDependentSpecializationInfo()) {
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
    if (ArgsAsWritten) {
      NIDS.reserve(NumArgsAsWritten);
      for (unsigned I = 0; I < NumArgsAsWritten; ++I) {
        if (auto ArgId = BuildNodeIdForTemplateArgument(ArgsAsWritten[I])) {
          NIDS.push_back(ArgId.value());
        } else {
          CouldGetAllTypes = false;
          break;
        }
      }
    } else {
      NIDS.reserve(Args->size());
      for (unsigned I = 0; I < Args->size(); ++I) {
        if (auto ArgId = BuildNodeIdForTemplateArgument(Args->get(I))) {
          NIDS.push_back(ArgId.value());
        } else {
          CouldGetAllTypes = false;
          break;
        }
      }
    }
    if (CouldGetAllTypes) {
      auto Confidence = TNsAreSpeculative
                            ? GraphObserver::Confidence::Speculative
                            : GraphObserver::Confidence::NonSpeculative;
      for (const auto& TN : TNs) {
        if (auto SpecializedNode = BuildNodeIdForTemplateName(TN.first)) {
          // Because partial specialization of function templates is forbidden,
          // instantiates edges will always choose the same type (a tapp with
          // the primary template as its first argument) as specializes edges.
          Observer.recordInstEdge(
              OuterNode,
              Observer.recordTappNode(SpecializedNode.value(), NIDS,
                                      NumArgsAsWritten),
              Confidence);
          Observer.recordSpecEdge(
              OuterNode,
              Observer.recordTappNode(SpecializedNode.value(), NIDS,
                                      NumArgsAsWritten),
              Confidence);
        }
      }
    }
  }
  SourceLocation DeclLoc = Decl->getLocation();
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  if (!DeclLoc.isMacroID() && Decl->isThisDeclarationADefinition() &&
      Decl->getBody() != nullptr) {
    Marks.set_marked_source_end(Decl->getBody()->getSourceRange().getBegin());
  } else {
    Marks.set_marked_source_end(
        NormalizeRange(Decl->getSourceRange()).getEnd());
  }
  Marks.set_name_range(NameRange);
  auto NameRangeInContext =
      RangeInCurrentContext(Decl->isImplicit(), OuterNode, NameRange);
  if (const auto* MF = dyn_cast<CXXMethodDecl>(Decl)) {
    // Use the record name token as the definition site for implicit member
    // functions, including implicit constructors and destructors.
    if (MF->isImplicit()) {
      if (const auto* Record = MF->getParent()) {
        if (!Record->isImplicit()) {
          NameRangeInContext = RangeInCurrentContext(
              false, OuterNode, RangeForNameOfDeclaration(MF));
        }
      }
    }
  }

  bool IsFunctionDefinition = IsDefinition(Decl);
  MaybeRecordDefinitionRange(NameRangeInContext, OuterNode,
                             BuildNodeIdForDefnOfDecl(Decl));

  if (IsFunctionDefinition && !Decl->isImplicit() &&
      Decl->getBody() != nullptr) {
    SourceRange DefinitionRange = NormalizeRange(
        {TemplateKeywordLoc.isValid() ? TemplateKeywordLoc
                                      : Decl->getSourceRange().getBegin(),
         Decl->getSourceRange().getEnd()});
    auto DefinitionRangeInContext =
        RangeInCurrentContext(Decl->isImplicit(), OuterNode, DefinitionRange);
    MaybeRecordFullDefinitionRange(DefinitionRangeInContext, OuterNode,
                                   BuildNodeIdForDefnOfDecl(Decl));
  }
  unsigned ParamNumber = 0;
  for (const auto* Param : Decl->parameters()) {
    ConnectParam(Decl, InnerNode, IsFunctionDefinition, ParamNumber++, Param,
                 IsImplicit);
  }

  absl::optional<GraphObserver::NodeId> FunctionType;
  if (auto* TSI = Decl->getTypeSourceInfo()) {
    FunctionType = BuildNodeIdForType(TSI->getTypeLoc());
  } else {
    FunctionType = BuildNodeIdForType(Decl->getType());
  }

  if (FunctionType) {
    Observer.recordTypeEdge(InnerNode, FunctionType.value());
  }

  if (const auto* MF = dyn_cast<CXXMethodDecl>(Decl)) {
    // OO_Call, OO_Subscript, and OO_Equal must be member functions.
    // The dyn_cast to CXXMethodDecl above is therefore not dropping
    // (impossible) free function incarnations of these operators from
    // consideration in the following.
    if (MF->size_overridden_methods() != 0) {
      for (auto O = MF->begin_overridden_methods(),
                E = MF->end_overridden_methods();
           O != E; ++O) {
        Observer.recordOverridesEdge(
            InnerNode,
            absl::GetFlag(FLAGS_experimental_alias_template_instantiations)
                ? BuildNodeIdForRefToDecl(*O)
                : BuildNodeIdForDecl(*O));
      }
      MapOverrideRoots(MF, [&](const CXXMethodDecl* R) {
        Observer.recordOverridesRootEdge(
            InnerNode,
            absl::GetFlag(FLAGS_experimental_alias_template_instantiations)
                ? BuildNodeIdForRefToDecl(R)
                : BuildNodeIdForDecl(R));
      });
    }
  }

  AddChildOfEdgeToDeclContext(Decl, OuterNode);
  GraphObserver::FunctionSubkind Subkind = GraphObserver::FunctionSubkind::None;
  if (const auto* CC = dyn_cast<CXXConstructorDecl>(Decl)) {
    Subkind = GraphObserver::FunctionSubkind::Constructor;
    size_t InitNumber = 0;
    for (const auto* Init : CC->inits()) {
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
          const SourceLocation& Loc = Init->getMemberLocation();
          if (Loc.isValid() && Loc.isFileID()) {
            MemberSR = NormalizeRange(Loc);
          }
          if (auto RCC = ExplicitRangeInCurrentContext(MemberSR)) {
            const auto& ID = BuildNodeIdForRefToDecl(M);
            Observer.recordDeclUseLocation(
                RCC.value(), ID, GraphObserver::Claimability::Claimable,
                this->IsImplicit(RCC.value()));
          }
        }
      }

      if (const auto* InitTy = Init->getTypeSourceInfo()) {
        auto QT = InitTy->getType().getCanonicalType();
        if (QT.getTypePtr()->isDependentType()) {
          if (auto TyId = BuildNodeIdForType(InitTy->getTypeLoc())) {
            auto DepName = Context.DeclarationNames.getCXXConstructorName(
                CanQualType::CreateUnsafe(QT));
            if (auto LookupId = RecordEdgesForDependentName(
                    clang::NestedNameSpecifierLoc(), DepName,
                    Init->getSourceLocation(), TyId)) {
              clang::SourceRange SR = NormalizeRange(Init->getSourceRange());
              if (Init->isWritten()) {
                if (auto RCC = ExplicitRangeInCurrentContext(SR)) {
                  RecordCallEdges(RCC.value(), LookupId.value());
                }
              } else {
                // clang::CXXCtorInitializer is its own special flavor of AST
                // node that needs extra care.
                auto InitIdent = LookupId.value().getRawIdentity() +
                                 std::to_string(InitNumber);
                auto InitId = GraphObserver::NodeId(LookupId.value().getToken(),
                                                    InitIdent);
                RecordCallEdges(GraphObserver::Range(InitId), LookupId.value());
              }
            }
          }
        }
      }
    }
  } else if (const auto* CD = dyn_cast<CXXDestructorDecl>(Decl)) {
    Subkind = GraphObserver::FunctionSubkind::Destructor;
  }
  if (!IsFunctionDefinition && Decl->getBuiltinID() == 0) {
    Observer.recordFunctionNode(InnerNode,
                                GraphObserver::Completeness::Incomplete,
                                Subkind, absl::nullopt);
    Observer.recordMarkedSource(OuterNode,
                                Marks.GenerateMarkedSource(OuterNode));
    AssignUSR(OuterNode, Decl);
    return true;
  }
  if (NameRangeInContext) {
    FileID DeclFile =
        Observer.getSourceManager()->getFileID(Decl->getLocation());
    for (const auto* NextDecl : Decl->redecls()) {
      const clang::Decl* OuterTemplate = nullptr;
      if (NextDecl != Decl) {
        const clang::Decl* OuterTemplate =
            NextDecl->getDescribedFunctionTemplate();
        FileID NextDeclFile =
            Observer.getSourceManager()->getFileID(NextDecl->getLocation());
        GraphObserver::NodeId TargetDecl =
            BuildNodeIdForDecl(OuterTemplate ? OuterTemplate : NextDecl);

        Observer.recordCompletionRange(
            NameRangeInContext.value(), TargetDecl,
            NextDeclFile == DeclFile
                ? GraphObserver::Specificity::UniquelyCompletes
                : GraphObserver::Specificity::Completes,
            OuterNode);
      }
    }
  }
  Observer.recordFunctionNode(InnerNode,
                              GraphObserver::Completeness::Definition, Subkind,
                              absl::nullopt);
  Observer.recordMarkedSource(OuterNode, Marks.GenerateMarkedSource(OuterNode));
  AssignUSR(OuterNode, Decl);
  return true;
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForTypedefNameDecl(
    const clang::TypedefNameDecl* Decl) {
  const auto Cached = DeclToNodeId.find(Decl);
  if (Cached != DeclToNodeId.end()) {
    return Cached->second;
  }
  clang::TypeSourceInfo* TSI = Decl->getTypeSourceInfo();
  if (auto AliasedTypeId = BuildNodeIdForType(TSI->getTypeLoc())) {
    auto Marks = MarkedSources.Generate(Decl);
    Marks.set_implicit(Job->UnderneathImplicitTemplateInstantiation);
    GraphObserver::NameId AliasNameId(BuildNameIdForDecl(Decl));
    AssignUSR(AliasNameId, AliasedTypeId.value(), Decl);
    return Observer.recordTypeAliasNode(
        AliasNameId, AliasedTypeId.value(),
        BuildNodeIdForType(FollowAliasChain(Decl)),
        Marks.GenerateMarkedSource(Observer.nodeIdForTypeAliasNode(
            AliasNameId, AliasedTypeId.value())));
  }
  return absl::nullopt;
}

bool IndexerASTVisitor::VisitObjCTypeParamDecl(
    const clang::ObjCTypeParamDecl* Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  GraphObserver::NodeId TypeParamId = BuildNodeIdForDecl(Decl);
  Observer.recordAbsVarNode(TypeParamId,
                            Marks.GenerateMarkedSource(TypeParamId));
  SourceRange TypeSR = RangeForNameOfDeclaration(Decl);
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), TypeParamId, TypeSR),
      TypeParamId, absl::nullopt);
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

  if (auto Type = BuildNodeIdForType(BoundInfo->getTypeLoc())) {
    if (Decl->hasExplicitBound()) {
      Observer.recordUpperBoundEdge(TypeParamId, Type.value());
    } else {
      Observer.recordTypeEdge(TypeParamId, Type.value());
    }
  }

  return true;
}

bool IndexerASTVisitor::VisitTypedefDecl(const clang::TypedefDecl* Decl) {
  return VisitCTypedef(Decl);
}

bool IndexerASTVisitor::VisitTypeAliasDecl(const clang::TypeAliasDecl* Decl) {
  return VisitCTypedef(Decl);
}

bool IndexerASTVisitor::VisitCTypedef(const clang::TypedefNameDecl* Decl) {
  if (Decl == Context.getBuiltinVaListDecl() ||
      Decl == Context.getInt128Decl() || Decl == Context.getUInt128Decl()) {
    // Don't index __uint128_t, __builtin_va_list, __int128_t
    return true;
  }
  if (auto InnerNodeId = BuildNodeIdForTypedefNameDecl(Decl)) {
    GraphObserver::NodeId OuterNodeId = InnerNodeId.value();
    // If this is a template, we need to emit an abs node for it.
    if (auto* TA = dyn_cast_or_null<TypeAliasDecl>(Decl)) {
      if (auto* TATD = TA->getDescribedAliasTemplate()) {
        OuterNodeId = RecordTemplate(TATD, InnerNodeId.value());
      }
    }
    SourceRange Range = RangeForNameOfDeclaration(Decl);
    MaybeRecordDefinitionRange(
        RangeInCurrentContext(Decl->isImplicit(), OuterNodeId, Range),
        OuterNodeId, absl::nullopt);
    AddChildOfEdgeToDeclContext(Decl, OuterNodeId);
    AssignUSR(OuterNodeId, Decl);
  }
  return true;
}

bool IndexerASTVisitor::VisitUsingShadowDecl(
    const clang::UsingShadowDecl* Decl) {
  if (auto RCC =
          ExplicitRangeInCurrentContext(RangeForNameOfDeclaration(Decl))) {
    Observer.recordDeclUseLocation(
        RCC.value(), BuildNodeIdForDecl(Decl->getTargetDecl()),
        GraphObserver::Claimability::Claimable, IsImplicit(RCC.value()));
  }
  return true;
}

GraphObserver::NameId::NameEqClass IndexerASTVisitor::BuildNameEqClassForDecl(
    const clang::Decl* D) const {
  CHECK(D != nullptr);
  if (const auto* T = dyn_cast<clang::TagDecl>(D)) {
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
  } else if (const auto* T = dyn_cast<clang::ClassTemplateDecl>(D)) {
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

absl::optional<GraphObserver::NodeId> IndexerASTVisitor::GetDeclChildOf(
    const clang::Decl* Decl) {
  for (const auto& Node : RootTraversal(getAllParents(), Decl)) {
    if (Node.indexed_parent == nullptr) break;

    // We would rather name 'template <etc> class C' as C, not C::C, but
    // we also want to be able to give useful names to templates when they're
    // explicitly requested. Therefore:
    if (Node.decl == nullptr || Node.decl == Decl ||
        isa<clang::ClassTemplateDecl>(Node.decl)) {
      continue;
    }
    if (const auto* ND = dyn_cast<clang::NamedDecl>(Node.decl)) {
      return BuildNodeIdForDecl(ND);
    }
  }
  return absl::nullopt;
}

/// \brief Attempts to add some representation of `ND` to `Ostream`.
/// \return true on success; false on failure.
bool IndexerASTVisitor::AddNameToStream(llvm::raw_string_ostream& Ostream,
                                        const clang::NamedDecl* ND) {
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
    } else if (const auto* RD = dyn_cast<clang::RecordDecl>(ND)) {
      Ostream << HashToString(Hash(RD));
    } else if (const auto* ED = dyn_cast<clang::EnumDecl>(ND)) {
      Ostream << HashToString(Hash(ED));
    } else if (const auto* MD = dyn_cast<clang::CXXMethodDecl>(ND)) {
      if (isa<clang::CXXConstructorDecl>(MD)) {
        return AddNameToStream(Ostream, MD->getParent());
      } else if (isa<clang::CXXDestructorDecl>(MD)) {
        Ostream << "~";
        // Append the parent name or a dependent index if the parent is
        // nameless.
        return AddNameToStream(Ostream, MD->getParent());
      } else if (const auto* CD = dyn_cast<clang::CXXConversionDecl>(MD)) {
        auto ToType = CD->getConversionType();
        if (ToType.isNull()) {
          return false;
        }
        ToType.print(Ostream,
                     clang::PrintingPolicy(*Observer.getLangOptions()));
        return true;
      }
    } else if (isObjCSelector(Name)) {
      Ostream << HashToString(Hash(Name.getObjCSelector()));
    } else {
      // Other NamedDecls-sans-names are given parent-dependent names.
      return false;
    }
  }
  return true;
}

void IndexerASTVisitor::AssignUSR(const GraphObserver::NodeId& TargetNode,
                                  const clang::NamedDecl* ND) {
  if (UsrByteSize <= 0 || Job->UnderneathImplicitTemplateInstantiation) return;
  const auto* DC = ND->getDeclContext();
  if (DC->isFunctionOrMethod()) return;
  llvm::SmallString<128> Usr;
  if (clang::index::generateUSRForDecl(ND, Usr)) return;
  Observer.assignUsr(TargetNode, Usr, UsrByteSize);
}

GraphObserver::NameId IndexerASTVisitor::BuildNameIdForDecl(
    const clang::Decl* Decl) {
  GraphObserver::NameId Id;
  Id.EqClass = BuildNameEqClassForDecl(Decl);
  if (!Verbosity && !const_cast<clang::Decl*>(Decl)->isLocalExternDecl() &&
      Decl->getParentFunctionOrMethod() != nullptr) {
    Id.Hidden = true;
  }
  // Cons onto the end of the name instead of the beginning to optimize for
  // prefix search.
  llvm::raw_string_ostream Ostream(Id.Path);
  bool MissingSeparator = false;
  for (const auto& Current : RootTraversal(getAllParents(), Decl)) {
    // TODO(zarko): Do we need to deal with nodes with no memoization data?
    // According to ASTTypeTrates.h:205, only Stmt, Decl, Type and
    // NestedNameSpecifier return memoization data. Can we claim an invariant
    // that if we start at any Decl, we will always encounter nodes with
    // memoization data?
    const IndexedParent* IP = Current.indexed_parent;
    if (IP == nullptr) {
      // Make sure that we don't miss out on implicit nodes.
      if (Current.decl && Current.decl->isImplicit()) {
        if (const NamedDecl* ND = dyn_cast<NamedDecl>(Current.decl)) {
          if (!AddNameToStream(Ostream, ND)) {
            if (const DeclContext* DC = ND->getDeclContext()) {
              if (DC->isFunctionOrMethod()) {
                // Heroically try to come up with a disambiguating identifier,
                // even when the IndexedParentVector is empty. This can happen
                // in anonymous parameter declarations that belong to function
                // prototypes.
                const clang::FunctionDecl* FD =
                    static_cast<const clang::FunctionDecl*>(DC);
                int param_count = 0, found_param = -1;
                for (const auto* P : FD->parameters()) {
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
    if (MissingSeparator && Current.decl &&
        isa<ClassTemplateDecl>(Current.decl)) {
      continue;
    }
    if (MissingSeparator && !dyn_cast_or_null<LinkageSpecDecl>(Current.decl)) {
      Ostream << ":";
    } else {
      MissingSeparator = true;
    }
    if (Current.decl) {
      // TODO(zarko): check for other specializations and emit accordingly
      // Alternately, maybe it would be better to just always emit the hash?
      // At any rate, a hash cache might be a good idea.
      if (const NamedDecl* ND = dyn_cast<NamedDecl>(Current.decl)) {
        if (!AddNameToStream(Ostream, ND)) {
          Ostream << IP->index;
        }
      } else if (const auto* LSD = dyn_cast<LinkageSpecDecl>(Current.decl)) {
        // Doing anything here breaks C headers that wrap extern "C" in
        // #ifdef __cplusplus.
      } else {
        // If there's no good name for this Decl, name it after its child
        // index wrt its parent node.
        Ostream << IP->index;
      }
    } else if (auto* S = Current.node.get<clang::Stmt>()) {
      // This is a Stmt--we can name it by its index wrt its parent node.
      Ostream << IP->index;
    }
  }
  Ostream.flush();
  return Id;
}

GraphObserver::NodeId IndexerASTVisitor::BuildNodeIdForDecl(
    const clang::Decl* Decl, unsigned Index) {
  GraphObserver::NodeId BaseId(BuildNodeIdForDecl(Decl));
  return GraphObserver::NodeId(
      BaseId.getToken(), BaseId.getRawIdentity() + "." + std::to_string(Index));
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForImplicitStmt(const clang::Stmt* Stmt) {
  if (!Verbosity) {
    return absl::nullopt;
  }
  // Do a quickish test to see if the Stmt is implicit.
  llvm::SmallVector<unsigned, 16> StmtPath;
  if (const clang::Decl* Decl =
          FindImplicitDeclForStmt(getAllParents(), Stmt, &StmtPath)) {
    auto DeclId = BuildNodeIdForDecl(Decl);
    std::string NewIdent = DeclId.getRawIdentity();
    {
      llvm::raw_string_ostream Ostream(NewIdent);
      for (auto& node : StmtPath) {
        Ostream << node << ".";
      }
    }
    return GraphObserver::NodeId(DeclId.getToken(), NewIdent);
  }
  return absl::nullopt;
}

GraphObserver::NodeId IndexerASTVisitor::BuildNodeIdForDecl(
    const clang::Decl* Decl) {
  // We can't assume that no two nodes of the same Kind can appear
  // simultaneously at the same SourceLocation: witness implicit overloaded
  // operator=. We rely on names and types to disambiguate them.
  // NodeIds must be stable across analysis runs with the same input data.
  // Some NodeIds are stable in the face of changes to that data, such as
  // the IDs given to class definitions (in part because of the language rules).

  if (absl::GetFlag(FLAGS_experimental_alias_template_instantiations)) {
    Decl = FindSpecializedTemplate(Decl);
  }

  if (const auto* IFD = dyn_cast<clang::IndirectFieldDecl>(Decl)) {
    // An IndirectFieldDecl is just an alias; we want to record this as a
    // reference to the underlying entity.
    Decl = IFD->getAnonField();
  }

  // find, not insert, since we might generate other IDs in the process of
  // generating this one (thus invalidating the iterator insert returns).
  const auto Cached = DeclToNodeId.find(Decl);
  if (Cached != DeclToNodeId.end()) {
    return Cached->second;
  }
  const auto* Token = Observer.getClaimTokenForLocation(Decl->getLocation());
  std::string Identity;
  llvm::raw_string_ostream Ostream(Identity);
  Ostream << BuildNameIdForDecl(Decl);

  // First, check to see if this thing is a builtin Decl. These things can
  // pick up weird declaration locations that aren't stable enough for us.
  if (const auto* FD = dyn_cast<FunctionDecl>(Decl)) {
    if (unsigned BuiltinID = FD->getBuiltinID()) {
      // If _FORTIFY_SOURCE is enabled, some builtin functions will grow
      // additional definitions (like fread in bits/stdio2.h). These
      // definitions have declarations that differ from their non-fortified
      // versions (in the sense that they're different sequences of tokens),
      // which leads us to generate different code facts for the same node.
      // This upsets our testing infrastructure. Similarly, some standard
      // libraries redeclare various builtin function with different aliased
      // names.  To workaround both of these, include all of the explicit
      // attributes in the ID unless this is the canonical decl.
      if (FD != FD->getCanonicalDecl()) {
        clang::AttrVec attrs;
        for (clang::Attr* attr : FD->attrs()) {
          if (!(attr->isImplicit() || attr->isInherited())) {
            attrs.push_back(attr);
          }
        }
        if (!attrs.empty()) {
          Ostream << absl::StrFormat(
              "#attrs(%s)",
              absl::StrJoin(attrs, "|",
                            [](std::string* out, const clang::Attr* attr) {
                              absl::StrAppend(out, attr->getSpelling());
                            }));
        }
      }
      Ostream << "#builtin";
      GraphObserver::NodeId Id(Observer.getClaimTokenForBuiltin(),
                               Ostream.str());
      DeclToNodeId.insert({Decl, Id});
      return Id;
    }
  }

  if (const auto* BTD = dyn_cast<BuiltinTemplateDecl>(Decl)) {
    Ostream << "#builtin";
    GraphObserver::NodeId Id(Observer.getClaimTokenForBuiltin(), Ostream.str());
    DeclToNodeId.insert({Decl, Id});
    return Id;
  }

  const TypedefNameDecl* TND;
  if ((TND = dyn_cast<TypedefNameDecl>(Decl)) &&
      !isa<ObjCTypeParamDecl>(Decl)) {
    // There's a special way to name type aliases but we want to handle type
    // parameters for Objective-C as "normal" named decls.
    if (auto TypedefNameId = BuildNodeIdForTypedefNameDecl(TND)) {
      DeclToNodeId.insert({Decl, TypedefNameId.value()});
      return TypedefNameId.value();
    }
  } else if (const auto* NS = dyn_cast<NamespaceDecl>(Decl)) {
    // Namespaces are named according to their NameIDs.
    Ostream << "#namespace";
    GraphObserver::NodeId Id(
        NS->isAnonymousNamespace()
            ? Observer.getAnonymousNamespaceClaimToken(NS->getLocation())
            : Observer.getNamespaceClaimToken(NS->getLocation()),
        Ostream.str());
    DeclToNodeId.insert({Decl, Id});
    return Id;
  }

  // Disambiguate nodes underneath template instances.
  for (const auto& Current : RootTraversal(getAllParents(), Decl)) {
    if (!Current.decl) continue;
    if (const auto* TD = dyn_cast<TemplateDecl>(Current.decl)) {
      // Disambiguate type abstraction IDs from abstracted type IDs.
      if (Current.decl != Decl) {
        Ostream << "#";
      }
    }
    if (!absl::GetFlag(FLAGS_experimental_alias_template_instantiations)) {
      if (const auto* CTSD =
              dyn_cast<ClassTemplateSpecializationDecl>(Current.decl)) {
        // Inductively, we can break after the first implicit instantiation*
        // (since its NodeId will contain its parent's first implicit
        // instantiation and so on). We still want to include hashes of
        // instantiation types.
        // * we assume that the first parent changing, if it does change, is not
        //   semantically important; we're generating stable internal IDs.
        if (Current.decl != Decl) {
          Ostream << "#" << BuildNodeIdForDecl(CTSD);
          if (CTSD->isImplicit()) {
            break;
          }
        } else {
          Ostream << "#"
                  << HashToString(Hash(&CTSD->getTemplateInstantiationArgs()));
        }
      } else if (const auto* FD = dyn_cast<FunctionDecl>(Current.decl)) {
        Ostream << "#"
                << HashToString(Hash(QualType(FD->getFunctionType(), 0)));
        if (const auto* TemplateArgs = FD->getTemplateSpecializationArgs()) {
          if (Current.decl != Decl) {
            Ostream << "#" << BuildNodeIdForDecl(FD);
            break;
          } else {
            Ostream << "#" << HashToString(Hash(TemplateArgs));
          }
        } else if (const auto* MSI = FD->getMemberSpecializationInfo()) {
          if (const auto* DC =
                  dyn_cast<const class Decl>(FD->getDeclContext())) {
            Ostream << "#" << BuildNodeIdForDecl(DC);
            break;
          }
        }
      } else if (const auto* VD =
                     dyn_cast<VarTemplateSpecializationDecl>(Current.decl)) {
        if (VD->isImplicit()) {
          if (Current.decl != Decl) {
            Ostream << "#" << BuildNodeIdForDecl(VD);
            break;
          } else {
            Ostream << "#"
                    << HashToString(Hash(&VD->getTemplateInstantiationArgs()));
          }
        }
      } else if (const auto* VD = dyn_cast<VarDecl>(Current.decl)) {
        if (const auto* MSI = VD->getMemberSpecializationInfo()) {
          if (const auto* DC =
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
  if (const auto* Rec = dyn_cast<clang::RecordDecl>(Decl)) {
    if (Rec->getDefinition() == Rec && Rec->getDeclName()) {
      Ostream << "#" << HashToString(Hash(Rec));
      GraphObserver::NodeId Id(Token, Ostream.str());
      DeclToNodeId.insert({Decl, Id});
      return Id;
    }
  } else if (const auto* Enum = dyn_cast<clang::EnumDecl>(Decl)) {
    if (Enum->getDefinition() == Enum) {
      Ostream << "#" << HashToString(Hash(Enum));
      GraphObserver::NodeId Id(Token, Ostream.str());
      DeclToNodeId.insert({Decl, Id});
      return Id;
    }
  } else if (const auto* ECD = dyn_cast<clang::EnumConstantDecl>(Decl)) {
    if (const auto* E = dyn_cast<clang::EnumDecl>(ECD->getDeclContext())) {
      if (E->getDefinition() == E) {
        Ostream << "#" << HashToString(Hash(E));
        GraphObserver::NodeId Id(Token, Ostream.str());
        DeclToNodeId.insert({Decl, Id});
        return Id;
      }
    }
  } else if (const auto* FD = dyn_cast<clang::FunctionDecl>(Decl)) {
    if (IsDefinition(FD)) {
      // TODO(zarko): Investigate why Clang colocates incomplete and
      // definition instances of FunctionDecls. This may have been masked
      // for enums and records because of the code above.
      Ostream << "#D";
    }
  } else if (const auto* VD = dyn_cast<clang::VarDecl>(Decl)) {
    if (IsDefinition(VD)) {
      // TODO(zarko): Investigate why Clang colocates incomplete and
      // definition instances of VarDecls. This may have been masked
      // for enums and records because of the code above.
      Ostream << "#D";
    }
  } else if (const auto* OD = dyn_cast<clang::ObjCInterfaceDecl>(Decl)) {
    if (OD->isThisDeclarationADefinition()) {
      // TODO(zarko): Investigate why Clang colocates incomplete and
      // definition instances of VarDecls. This may have been masked
      // for enums and records because of the code above.
      Ostream << "#D";
    }
  }
  clang::SourceRange DeclRange;
  if (const auto* named_decl = dyn_cast<NamedDecl>(Decl)) {
    DeclRange = RangeForNameOfDeclaration(named_decl);
  }
  if (!DeclRange.isValid()) {
    DeclRange = clang::SourceRange(Decl->getBeginLoc(), Decl->getEndLoc());
  }
  Ostream << "@";
  if (DeclRange.getBegin().isValid()) {
    Observer.AppendRangeToStream(
        Ostream, GraphObserver::Range(
                     DeclRange, Observer.getClaimTokenForRange(DeclRange)));
  } else {
    Ostream << "invalid";
  }
  GraphObserver::NodeId Id(Token, Ostream.str());
  DeclToNodeId.insert({Decl, Id});
  return Id;
}

bool IndexerASTVisitor::IsDefinition(const FunctionDecl* FunctionDecl) {
  return FunctionDecl->isThisDeclarationADefinition();
}

GraphObserver::NodeId IndexerASTVisitor::ApplyBuiltinTypeConstructor(
    const char* BuiltinName, const GraphObserver::NodeId& Param) {
  GraphObserver::NodeId TyconID = Observer.getNodeIdForBuiltinType(BuiltinName);
  return Observer.recordTappNode(TyconID, {Param});
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForTemplateName(const clang::TemplateName& Name) {
  // TODO(zarko): Do we need to canonicalize `Name`?
  // Maybe with Context.getCanonicalTemplateName()?
  switch (Name.getKind()) {
    case TemplateName::Template: {
      const TemplateDecl* TD = Name.getAsTemplateDecl();
      if (const auto* TTPD = dyn_cast<TemplateTemplateParmDecl>(TD)) {
        return BuildNodeIdForDecl(TTPD);
      } else if (const NamedDecl* UnderlyingDecl = TD->getTemplatedDecl()) {
        if (const auto* TD = dyn_cast<TypeDecl>(UnderlyingDecl)) {
          // TODO(zarko): Under which circumstances is this nullptr?
          // Should we treat this as a type here or as a decl?
          // We've already made the decision elsewhere to link to class
          // definitions directly (in place of nominal nodes), so calling
          // BuildNodeIdForDecl() all the time makes sense. We aren't even
          // emitting ranges.
          if (const auto* TDType = TD->getTypeForDecl()) {
            return BuildNodeIdForType(QualType(TDType, 0));
          } else if (const auto* TAlias = dyn_cast<TypeAliasDecl>(TD)) {
            // The names for type alias types are the same for type alias nodes.
            return BuildNodeIdForTypedefNameDecl(TAlias);
          } else {
            CHECK(IgnoreUnimplemented)
                << "Unknown case in BuildNodeIdForTemplateName";
            return absl::nullopt;
          }
        } else if (const auto* FD = dyn_cast<FunctionDecl>(UnderlyingDecl)) {
          // Direct references to function templates to the outer function
          // template shell.
          return BuildNodeIdForDecl(Name.getAsTemplateDecl());
        } else if (const auto* VD = dyn_cast<VarDecl>(UnderlyingDecl)) {
          // Direct references to variable templates to the appropriate
          // template decl (may be a partial specialization or the
          // primary template).
          return BuildNodeIdForDecl(Name.getAsTemplateDecl());
        } else {
          LOG(FATAL) << "Unexpected UnderlyingDecl";
        }
      } else if (const auto* BTD = dyn_cast<BuiltinTemplateDecl>(TD)) {
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
  return absl::nullopt;
}

bool IndexerASTVisitor::TraverseNestedNameSpecifierLoc(
    clang::NestedNameSpecifierLoc NNS) {
  if (!NNS) {
    return true;
  }
  if (!Verbosity) {
    return Base::TraverseNestedNameSpecifierLoc(NNS);
  }
  switch (NNS.getNestedNameSpecifier()->getKind()) {
    case NestedNameSpecifier::TypeSpec:
      break;  // This is handled by VisitDependentNameTypeLoc.
    case NestedNameSpecifier::Identifier: {
      auto DId = BuildNodeIdForDependentIdentifier(
          NNS.getPrefix().getNestedNameSpecifier(),
          NNS.getNestedNameSpecifier()->getAsIdentifier());
      if (auto RCC = ExplicitRangeInCurrentContext(NNS.getLocalSourceRange())) {
        Observer.recordDeclUseLocation(*RCC, DId,
                                       GraphObserver::Claimability::Claimable,
                                       IsImplicit(*RCC));
      }
    } break;
    default:
      break;
  }
  return Base::TraverseNestedNameSpecifierLoc(NNS);
}

GraphObserver::NodeId IndexerASTVisitor::BuildNodeIdForDependentIdentifier(
    const clang::NestedNameSpecifier* Prefix,
    const clang::IdentifierInfo* Identifier) {
  return BuildNodeIdForDependentName(Prefix,
                                     clang::DeclarationName(Identifier));
}

GraphObserver::NodeId IndexerASTVisitor::BuildNodeIdForDependentName(
    const clang::NestedNameSpecifier* Prefix,
    const clang::DeclarationName& Identifier) {
  // TODO(zarko): Need a better way to generate stablish names here.
  // In particular, it would be nice if a dependent name A::B::C
  // and a dependent name A::B::D were represented as ::C and ::D off
  // of the same dependent root A::B. (Does this actually make sense,
  // though? Could A::B resolve to a different entity in each case?)
  std::string Identity = "#nns::";  // Nested name specifier.

  llvm::raw_string_ostream Ostream(Identity);
  if (auto PId = BuildNodeIdForNestedNameSpecifier(Prefix)) {
    Ostream << PId->IdentityRef();
  } else {
    Ostream << "(invalid)";
  }
  Ostream << "::";
  if (auto Name = GetDeclarationName(Identifier, IgnoreUnimplemented)) {
    Ostream << *Name;
  } else {
    Ostream << "(invalid)";
  }
  return GraphObserver::NodeId(Observer.getDefaultClaimToken(), Ostream.str());
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::RecordParamEdgesForDependentName(
    const GraphObserver::NodeId& DId, clang::NestedNameSpecifierLoc NNSLoc,
    const absl::optional<GraphObserver::NodeId>& Root) {
  unsigned SubIdCount = 0;
  if (!NNSLoc && Root) {
    Observer.recordParamEdge(DId, SubIdCount++, *Root);
  }
  for (; NNSLoc; NNSLoc = NNSLoc.getPrefix()) {
    const clang::NestedNameSpecifier* NNS = NNSLoc.getNestedNameSpecifier();
    switch (NNS->getKind()) {
      case NestedNameSpecifier::Identifier:
        // Hashcons the identifiers.
        if (auto Subtree = RecordParamEdgesForDependentName(
                BuildNodeIdForDependentIdentifier(NNS->getPrefix(),
                                                  NNS->getAsIdentifier()),
                NNSLoc.getPrefix(), Root)) {
          if (RecordLookupEdgeForDependentName(*Subtree,
                                               NNS->getAsIdentifier())) {
            Observer.recordParamEdge(DId, SubIdCount++, *Subtree);
            return DId;
          }
        }
        CHECK(IgnoreUnimplemented) << "NNS::Identifier";
        return absl::nullopt;
      case NestedNameSpecifier::Namespace:
        // TODO(zarko): Emit some representation to back this node ID.
      default:
        if (auto SubId = BuildNodeIdForNestedNameSpecifierLoc(NNSLoc)) {
          Observer.recordParamEdge(DId, SubIdCount++, *SubId);
        } else {
          return absl::nullopt;
        }
    }
  }
  return DId;
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::RecordLookupEdgeForDependentName(
    const GraphObserver::NodeId& DId, const clang::DeclarationName& Name) {
  if (auto Lookup = GetDeclarationName(Name, IgnoreUnimplemented)) {
    Observer.recordLookupNode(DId, *Lookup);
    return DId;
  }
  return absl::nullopt;
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForNestedNameSpecifierLoc(
    const clang::NestedNameSpecifierLoc& NNSLoc) {
  if (!NNSLoc) {
    return absl::nullopt;
  }
  return BuildNodeIdForNestedNameSpecifier(NNSLoc.getNestedNameSpecifier());
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForNestedNameSpecifier(
    const clang::NestedNameSpecifier* NNS) {
  if (!NNS) {
    return absl::nullopt;
  }
  switch (NNS->getKind()) {
    case NestedNameSpecifier::Identifier:
      return BuildNodeIdForDependentIdentifier(NNS->getPrefix(),
                                               NNS->getAsIdentifier());
    case NestedNameSpecifier::Namespace:
      return BuildNodeIdForDecl(NNS->getAsNamespace());
    case NestedNameSpecifier::NamespaceAlias:
      return BuildNodeIdForDecl(NNS->getAsNamespaceAlias());
    case NestedNameSpecifier::TypeSpec:
      if (auto SubId =
              BuildNodeIdForType(clang::QualType(NNS->getAsType(), 0))) {
        return SubId;
      }
      return absl::nullopt;
    case NestedNameSpecifier::TypeSpecWithTemplate:
      CHECK(IgnoreUnimplemented) << "NNS::TypeSpecWithTemplate";
      return absl::nullopt;
    case NestedNameSpecifier::Global:
      CHECK(IgnoreUnimplemented) << "NNS::Global";
      return absl::nullopt;
    case NestedNameSpecifier::Super:
      CHECK(IgnoreUnimplemented) << "NNS::Super";
      return absl::nullopt;
  }
  CHECK(IgnoreUnimplemented) << "Unexpected NestedNameSpecifier kind.";
  return absl::nullopt;
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::RecordEdgesForDependentName(
    const clang::NestedNameSpecifierLoc& NNSLoc,
    const clang::DeclarationName& Id, const clang::SourceLocation IdLoc,
    const absl::optional<GraphObserver::NodeId>& Root) {
  if (!Verbosity) {
    return absl::nullopt;
  }
  if (auto IdOut = RecordParamEdgesForDependentName(
          BuildNodeIdForDependentName(NNSLoc.getNestedNameSpecifier(), Id),
          NNSLoc, Root)) {
    return RecordLookupEdgeForDependentName(*IdOut, Id);
  }
  return absl::nullopt;
}

GraphObserver::NodeId IndexerASTVisitor::BuildNodeIdForSpecialTemplateArgument(
    llvm::StringRef Id) {
  std::string Identity;
  llvm::raw_string_ostream Ostream(Identity);
  Ostream << Id << "#sta";
  return GraphObserver::NodeId(Observer.getDefaultClaimToken(), Ostream.str());
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForTemplateExpansion(clang::TemplateName Name) {
  return BuildNodeIdForTemplateName(Name);
}

absl::optional<GraphObserver::NodeId> IndexerASTVisitor::BuildNodeIdForExpr(
    const clang::Expr* Expr, EmitRanges ER) {
  if (!Verbosity || Expr == nullptr) {
    return absl::nullopt;
  }
  clang::Expr::EvalResult Result;
  std::string Identity;
  llvm::raw_string_ostream Ostream(Identity);
  std::string Text;
  llvm::raw_string_ostream TOstream(Text);
  bool IsBindingSite = false;
  auto RCC = RangeInCurrentContext(BuildNodeIdForImplicitStmt(Expr),
                                   NormalizeRange(Expr->getExprLoc()));
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
      return absl::nullopt;
    }
    Observer.AppendRangeToStream(Ostream, RCC.value());
    Expr->printPretty(TOstream, nullptr,
                      clang::PrintingPolicy(*Observer.getLangOptions()));
    Ostream << TOstream.str();
    IsBindingSite = true;
  }
  auto ResultId =
      GraphObserver::NodeId(Observer.getDefaultClaimToken(), Ostream.str());
  if (ER == EmitRanges::Yes && RCC) {
    if (IsBindingSite) {
      Observer.recordDefinitionBindingRange(RCC.value(), ResultId);
      Observer.recordLookupNode(ResultId, TOstream.str());
    } else {
      Observer.recordDeclUseLocation(RCC.value(), ResultId,
                                     GraphObserver::Claimability::Claimable,
                                     IsImplicit(RCC.value()));
    }
  }
  return ResultId;
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForTemplateArgument(
    const clang::TemplateArgument& Arg) {
  // TODO(zarko): Do we need to canonicalize `Arg`?
  // Maybe with Context.getCanonicalTemplateArgument()?
  switch (Arg.getKind()) {
    case TemplateArgument::Null:
      return BuildNodeIdForSpecialTemplateArgument("null");
    case TemplateArgument::Type:
      CHECK(!Arg.getAsType().isNull());
      return BuildNodeIdForType(Arg.getAsType());
    case TemplateArgument::Declaration:
      return BuildNodeIdForDecl(Arg.getAsDecl());
    case TemplateArgument::NullPtr:
      return BuildNodeIdForSpecialTemplateArgument("nullptr");
    case TemplateArgument::Integral:
      return BuildNodeIdForSpecialTemplateArgument(
          Arg.getAsIntegral().toString(10) + "i");
    case TemplateArgument::Template:
      return BuildNodeIdForTemplateName(Arg.getAsTemplate());
    case TemplateArgument::TemplateExpansion:
      return BuildNodeIdForTemplateExpansion(
          Arg.getAsTemplateOrTemplatePattern());
    case TemplateArgument::Expression:
      CHECK(Arg.getAsExpr() != nullptr);
      return BuildNodeIdForExpr(Arg.getAsExpr(), EmitRanges::Yes);
    case TemplateArgument::Pack: {
      std::vector<GraphObserver::NodeId> Nodes;
      Nodes.reserve(Arg.pack_size());
      for (const auto& Element : Arg.pack_elements()) {
        auto Id = BuildNodeIdForTemplateArgument(Element);
        if (!Id) {
          return absl::nullopt;
        }
        Nodes.push_back(Id.value());
      }
      return Observer.recordTsigmaNode(Nodes);
    }
  }
  return absl::nullopt;
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForTemplateArgument(
    const clang::TemplateArgumentLoc& ArgLoc) {
  return BuildNodeIdForTemplateArgument(ArgLoc.getArgument());
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

NodeSet IndexerASTVisitor::BuildNodeSetForBuiltin(
    const clang::BuiltinType& T) const {
  return Observer.getNodeIdForBuiltinType(
      T.getName(clang::PrintingPolicy(*Observer.getLangOptions())));
}

NodeSet IndexerASTVisitor::BuildNodeSetForEnum(const clang::EnumType& T) {
  EnumDecl* Decl = T.getDecl();
  if (EnumDecl* Defn = Decl->getDefinition()) {
    return {BuildNodeIdForDecl(Defn), GraphObserver::Claimability::Unclaimable};
  }
  // If there is no visible definition, Id will be a tnominal node
  // whereas it is more useful to decorate the span as a reference
  // to the visible declaration.
  // See https://github.com/kythe/kythe/issues/2329
  return {BuildNominalNodeIdForDecl(Decl), BuildNodeIdForDecl(Decl)};
}

NodeSet IndexerASTVisitor::BuildNodeSetForRecord(const clang::RecordType& T) {
  RecordDecl* Decl = CHECK_NOTNULL(T.getDecl());
  if (const auto* Spec = dyn_cast<ClassTemplateSpecializationDecl>(Decl)) {
    // TODO(shahms): Simplify building template argument lists.
    const auto& TAL = Spec->getTemplateArgs();
    std::vector<GraphObserver::NodeId> TemplateArgs;
    TemplateArgs.reserve(TAL.size());
    for (const auto& Arg : TAL.asArray()) {
      if (auto ArgA = BuildNodeIdForTemplateArgument(Arg)) {
        TemplateArgs.push_back(ArgA.value());
      } else {
        return NodeSet::Empty();
      }
    }
    const auto* SpecDecl = Spec->getSpecializedTemplate();
    NodeId DeclId =
        Observer.recordTappNode(BuildNodeIdForDecl(SpecDecl), TemplateArgs);
    if (SpecDecl->getTemplatedDecl()->getDefinition()) {
      return {DeclId, Claimability::Unclaimable};
    } else {
      return {Observer.recordTappNode(BuildNominalNodeIdForDecl(SpecDecl),
                                      TemplateArgs),
              DeclId};
    }
  } else {
    return BuildNodeSetForNonSpecializedRecordDecl(Decl);
  }
}

NodeSet IndexerASTVisitor::BuildNodeSetForInjectedClassName(
    const clang::InjectedClassNameType& T) {
  // TODO(zarko): Replace with logic that uses InjectedType.
  return BuildNodeSetForNonSpecializedRecordDecl(T.getDecl());
}

NodeSet IndexerASTVisitor::BuildNodeSetForTemplateTypeParm(
    const clang::TemplateTypeParmType& T) {
  if (const auto* Decl = FindTemplateTypeParmTypeDecl(T)) {
    return BuildNodeIdForDecl(Decl);
  }
  return NodeSet::Empty();
}

NodeSet IndexerASTVisitor::BuildNodeSetForPointer(const clang::PointerType& T) {
  if (auto PointeeID = BuildNodeIdForType(T.getPointeeType())) {
    return ApplyBuiltinTypeConstructor("ptr", *PointeeID);
  }
  return NodeSet::Empty();
}

NodeSet IndexerASTVisitor::BuildNodeSetForMemberPointer(
    const clang::MemberPointerType& T) {
  if (auto PointeeID = BuildNodeIdForType(T.getPointeeType())) {
    if (auto ClassID = BuildNodeIdForType(T.getClass())) {
      auto tapp = Observer.getNodeIdForBuiltinType("mptr");
      return Observer.recordTappNode(tapp, {*PointeeID, *ClassID});
    }
  }
  return NodeSet::Empty();
}

NodeSet IndexerASTVisitor::BuildNodeSetForLValueReference(
    const clang::LValueReferenceType& T) {
  if (auto PointeeID = BuildNodeIdForType(T.getPointeeType())) {
    return ApplyBuiltinTypeConstructor("lvr", *PointeeID);
  }
  return NodeSet::Empty();
}

NodeSet IndexerASTVisitor::BuildNodeSetForRValueReference(
    const clang::RValueReferenceType& T) {
  if (auto PointeeID = BuildNodeIdForType(T.getPointeeType())) {
    return ApplyBuiltinTypeConstructor("rvr", *PointeeID);
  }
  return NodeSet::Empty();
}

NodeSet IndexerASTVisitor::BuildNodeSetForAuto(const clang::AutoType& T) {
  return BuildNodeSetForDeduced(T);
}

NodeSet IndexerASTVisitor::BuildNodeSetForDeducedTemplateSpecialization(
    const clang::DeducedTemplateSpecializationType& T) {
  // TODO(shahms): This should look more like TemplateSpecialization than Auto.
  // They currently return the same thing, but only via the indirection through
  // the deduced type.  We should also potentially emit implicit references to
  // the deduced template parameters.
  return BuildNodeSetForDeduced(T);
}

NodeSet IndexerASTVisitor::BuildNodeSetForDeduced(const clang::DeducedType& T) {
  auto DeducedQT = T.getDeducedType();
  if (DeducedQT.isNull()) {
    // We still need to come up with a name here--it's more useful than
    // returning None, since we might be down a branch of some structural
    // type. We might also have an unconstrained type variable,
    // as with `auto foo();` with no definition.
    // TODO(zarko): Is "auto" the correct thing to return here for
    // a DeducedTemplateSpecialization?
    return Observer.getNodeIdForBuiltinType("auto");
  }
  return BuildNodeSetForType(DeducedQT);
}

NodeSet IndexerASTVisitor::BuildNodeSetForConstantArray(
    const clang::ConstantArrayType& T) {
  if (auto ElementID = BuildNodeIdForType(T.getElementType())) {
    // TODO(zarko): Record size expression.
    return ApplyBuiltinTypeConstructor("carr", *ElementID);
  }
  return NodeSet::Empty();
}

NodeSet IndexerASTVisitor::BuildNodeSetForIncompleteArray(
    const clang::IncompleteArrayType& T) {
  if (auto ElementID = BuildNodeIdForType(T.getElementType())) {
    return ApplyBuiltinTypeConstructor("iarr", *ElementID);
  }
  return NodeSet::Empty();
}

NodeSet IndexerASTVisitor::BuildNodeSetForDependentSizedArray(
    const clang::DependentSizedArrayType& T) {
  if (auto ElemID = BuildNodeIdForType(T.getElementType())) {
    if (auto ExprID = BuildNodeIdForExpr(T.getSizeExpr(), EmitRanges::No)) {
      return Observer.recordTappNode(Observer.getNodeIdForBuiltinType("darr"),
                                     {*ElemID, *ExprID});
    } else {
      return ApplyBuiltinTypeConstructor("darr", *ElemID);
    }
  }
  return NodeSet::Empty();
}

NodeSet IndexerASTVisitor::BuildNodeSetForExtInt(const clang::ExtIntType& T) {
  return Observer.getNodeIdForBuiltinType(absl::StrCat(
      T.isUnsigned() ? "unsigned _ExtInt" : "_ExtInt", "#", T.getNumBits()));
}

NodeSet IndexerASTVisitor::BuildNodeSetForDependentExtInt(
    const clang::DependentExtIntType& T) {
  if (auto ExprID = BuildNodeIdForExpr(T.getNumBitsExpr(), EmitRanges::No)) {
    return ApplyBuiltinTypeConstructor(
        T.isUnsigned() ? "unsigned _ExtInt" : "_ExtInt", *ExprID);
  }
  return NodeSet::Empty();
}

NodeSet IndexerASTVisitor::BuildNodeSetForFunctionProto(
    const clang::FunctionProtoType& T) {
  std::vector<GraphObserver::NodeId> NodeIds;
  auto ReturnType = BuildNodeIdForType(T.getReturnType());
  if (!ReturnType) {
    return NodeSet::Empty();
  }
  NodeIds.push_back(*ReturnType);

  for (const auto& param : T.getParamTypes()) {
    if (auto ParmType = BuildNodeIdForType(param)) {
      NodeIds.push_back(*ParmType);
    } else {
      return NodeSet::Empty();
    }
  }

  const char* Tycon = T.isVariadic() ? "fnvararg" : "fn";
  return Observer.recordTappNode(Observer.getNodeIdForBuiltinType(Tycon),
                                 NodeIds);
}

NodeSet IndexerASTVisitor::BuildNodeSetForFunctionNoProto(
    const clang::FunctionNoProtoType& T) {
  return Observer.getNodeIdForBuiltinType("knrfn");
}

NodeSet IndexerASTVisitor::BuildNodeSetForParen(const clang::ParenType& T) {
  return BuildNodeSetForType(T.getInnerType());
}

NodeSet IndexerASTVisitor::BuildNodeSetForDecltype(
    const clang::DecltypeType& T) {
  return BuildNodeSetForType(T.getUnderlyingType());
}

NodeSet IndexerASTVisitor::BuildNodeSetForElaborated(
    const clang::ElaboratedType& T) {
  return BuildNodeSetForType(T.getNamedType());
}

NodeSet IndexerASTVisitor::BuildNodeSetForTypedef(const clang::TypedefType& T) {
  // TODO(zarko): Return canonicalized versions as well.
  GraphObserver::NameId AliasID = BuildNameIdForDecl(T.getDecl());
  // We're retrieving the type of an alias here, so we shouldn't thread
  // through the deduced type.
  if (auto AliasedTypeID =
          BuildNodeIdForType(T.getDecl()->getTypeSourceInfo()->getTypeLoc())) {
    // TODO(shahms): Move caching here and use
    // `Observer.nodeIdForTypeAliasNode()` when already built?
    // Or is always using the cached id sufficient?
    NodeId ID = Observer.nodeIdForTypeAliasNode(AliasID, *AliasedTypeID);
    auto Marks = MarkedSources.Generate(T.getDecl());
    AssignUSR(ID, T.getDecl());
    return Observer.recordTypeAliasNode(
        ID, *AliasedTypeID, BuildNodeIdForType(FollowAliasChain(T.getDecl())),
        Marks.GenerateMarkedSource(ID));
  }
  return NodeSet::Empty();
}

NodeSet IndexerASTVisitor::BuildNodeSetForSubstTemplateTypeParm(
    const clang::SubstTemplateTypeParmType& T) {
  return BuildNodeSetForType(T.getReplacementType());
}

NodeSet IndexerASTVisitor::BuildNodeSetForDependentName(
    const clang::DependentNameType& T) {
  return BuildNodeIdForDependentIdentifier(T.getQualifier(), T.getIdentifier());
}

NodeSet IndexerASTVisitor::BuildNodeSetForTemplateSpecialization(
    const clang::TemplateSpecializationType& T) {
  // This refers to a particular class template, type alias template,
  // or template template parameter. Non-dependent template
  // specializations appear as different types.
  if (auto TemplateName = BuildNodeIdForTemplateName(T.getTemplateName())) {
    std::vector<GraphObserver::NodeId> TemplateArgs;
    TemplateArgs.reserve(T.getNumArgs());
    for (const auto& arg : T.template_arguments()) {
      if (auto ArgId = BuildNodeIdForTemplateArgument(arg)) {
        TemplateArgs.push_back(*ArgId);
      } else {
        return NodeSet::Empty();
      }
    }
    return Observer.recordTappNode(*TemplateName, TemplateArgs);
  }
  return NodeSet::Empty();
}

NodeSet IndexerASTVisitor::BuildNodeSetForPackExpansion(
    const clang::PackExpansionType& T) {
  return BuildNodeSetForType(T.getPattern());
}

NodeSet IndexerASTVisitor::BuildNodeSetForBlockPointer(
    const clang::BlockPointerType& T) {
  if (auto ID = BuildNodeIdForType(T.getPointeeType())) {
    return ApplyBuiltinTypeConstructor("ptr", *ID);
  }
  return NodeSet::Empty();
}

NodeSet IndexerASTVisitor::BuildNodeSetForObjCObjectPointer(
    const clang::ObjCObjectPointerType& T) {
  if (auto ID = BuildNodeIdForType(T.getPointeeType())) {
    return ApplyBuiltinTypeConstructor("ptr", *ID);
  }
  return NodeSet::Empty();
}

NodeSet IndexerASTVisitor::BuildNodeSetForObjCObject(
    const clang::ObjCObjectType& T) {
  if (auto BaseId = BuildNodeIdForObjCProtocols(T)) {
    if (T.getTypeArgsAsWritten().size() == 0) {
      return *std::move(BaseId);
    }
    std::vector<NodeId> GenericArgIds;
    GenericArgIds.reserve(T.getTypeArgsAsWritten().size());
    for (const auto& QT : T.getTypeArgsAsWritten()) {
      if (auto Arg = BuildNodeIdForType(QT)) {
        GenericArgIds.push_back(*Arg);
      } else {
        return NodeSet::Empty();
      }
    }
    return Observer.recordTappNode(*BaseId, GenericArgIds);
  }
  return NodeSet::Empty();
}

NodeSet IndexerASTVisitor::BuildNodeSetForObjCTypeParam(
    const clang::ObjCTypeParamType& T) {
  if (const auto* Decl = T.getDecl()) {
    return BuildNodeIdForDecl(Decl);
  }
  return NodeSet::Empty();
}

NodeSet IndexerASTVisitor::BuildNodeSetForObjCInterface(
    const clang::ObjCInterfaceType& T) {
  const auto* IFace = CHECK_NOTNULL(T.getDecl());
  // Link to the implementation if we have one, otherwise link to the
  // interface. If we just have a forward declaration, link to the nominal
  // type node.
  if (const auto* Impl = IFace->getImplementation()) {
    return {BuildNodeIdForDecl(Impl), Claimability::Unclaimable};
  } else if (!IsObjCForwardDecl(IFace)) {
    return {BuildNodeIdForDecl(IFace), Claimability::Unclaimable};
  } else {
    // Thanks to the ODR, we shouldn't record multiple nominal type nodes
    // for the same TU: given distinct names, NameIds will be distinct,
    // there may be only one definition bound to each name, and we
    // memoize the NodeIds we give to types.
    return BuildNominalNodeIdForDecl(IFace);
  }
}

NodeSet IndexerASTVisitor::BuildNodeSetForAttributed(
    const clang::AttributedType& T) {
  return BuildNodeSetForType(T.getModifiedType());
}

NodeSet IndexerASTVisitor::BuildNodeSetForDependentAddressSpace(
    const clang::DependentAddressSpaceType& T) {
  return BuildNodeSetForType(T.getPointeeType());
}

GraphObserver::NodeId IndexerASTVisitor::BuildNominalNodeIdForDecl(
    const clang::NamedDecl* Decl) {
  auto NominalID = Observer.nodeIdForNominalTypeNode(BuildNameIdForDecl(Decl));
  return Observer.recordNominalTypeNode(
      NominalID, MarkedSources.Generate(Decl).GenerateMarkedSource(NominalID),
      GetDeclChildOf(Decl));
}

NodeSet IndexerASTVisitor::BuildNodeSetForNonSpecializedRecordDecl(
    const clang::RecordDecl* Decl) {
  if (const RecordDecl* Defn = Decl->getDefinition()) {
    // Special-case linking to a defn instead of using a tnominal.
    if (const auto* RD = dyn_cast<CXXRecordDecl>(Defn)) {
      if (const auto* CTD = RD->getDescribedClassTemplate()) {
        // Link to the template binder, not the internal class.
        return {BuildNodeIdForDecl(CTD), Claimability::Unclaimable};
      }
    }
    // This definition is a non-CXXRecordDecl or non-template.
    return {BuildNodeIdForDecl(Defn), Claimability::Unclaimable};
  } else if (Decl->isEmbeddedInDeclarator()) {
    // Still use tnominal refs for C-style "struct foo* bar"
    // declarations.
    // TODO(zarko): Add completions for these.
    return BuildNominalNodeIdForDecl(Decl);
  } else {
    // If there is no visible definition, Id will be a tnominal node
    // whereas it is more useful to decorate the span as a reference
    // to the visible declaration.
    // See https://github.com/kythe/kythe/issues/2329
    return {BuildNominalNodeIdForDecl(Decl), BuildNodeIdForDecl(Decl)};
  }
}

const clang::TemplateTypeParmDecl*
IndexerASTVisitor::FindTemplateTypeParmTypeDecl(
    const clang::TemplateTypeParmType& T) const {
  // Either the `TemplateTypeParm` will link directly to a relevant
  // `TemplateTypeParmDecl` or (particularly in the case of canonicalized
  // types) we will find the Decl in the `Job->TypeContext` according to the
  // parameter's depth and index.
  // Depths count from the outside-in; each Template*ParmDecl has only
  // one possible (depth, index).
  if (auto* Decl = T.getDecl()) {
    return Decl;
  }
  LOG(INFO) << "Immediate TemplateTypeParmDecl not found, falling back to "
               "TypeContext";
  if (T.getDepth() < Job->TypeContext.size() &&
      T.getIndex() < Job->TypeContext[T.getDepth()]->size()) {
    return cast<clang::TemplateTypeParmDecl>(
        Job->TypeContext[T.getDepth()]->getParam(T.getIndex()));
  }
  LOG(ERROR) << "Unable to find TemplateTypeParmDecl for TemplateTypeParmType";
  return nullptr;
}

absl::optional<NodeId> IndexerASTVisitor::BuildNodeIdForType(
    const clang::TypeLoc& TypeLoc) {
  return BuildNodeSetForType(TypeLoc).AsOptional();
}

absl::optional<NodeId> IndexerASTVisitor::BuildNodeIdForType(
    const clang::QualType& QT) {
  return BuildNodeSetForType(QT).AsOptional();
}

absl::optional<NodeId> IndexerASTVisitor::BuildNodeIdForType(
    const clang::Type* T) {
  return BuildNodeSetForType(T).AsOptional();
}

NodeSet IndexerASTVisitor::BuildNodeSetForType(const clang::Type* T) {
  CHECK(T != nullptr);
  return BuildNodeSetForType(clang::QualType(T, 0));
}

NodeSet IndexerASTVisitor::BuildNodeSetForType(const clang::TypeLoc& TL) {
  return BuildNodeSetForType(TL.getType());
}

NodeSet IndexerASTVisitor::BuildNodeSetForType(const clang::QualType& QT) {
  CHECK(!QT.isNull());
  TypeKey Key(Context, QT, QT.getTypePtr());
  auto [iter, inserted] = TypeNodes.insert({Key, NodeSet::Empty()});
  if (inserted) {
    iter->second = QT.hasLocalQualifiers()
                       ? BuildNodeSetForTypeInternal(QT)
                       : BuildNodeSetForTypeInternal(*QT.getTypePtr());
  }
  return iter->second;
}

NodeSet IndexerASTVisitor::BuildNodeSetForTypeInternal(
    const clang::QualType& QT) {
  CHECK(!QT.isNull() && QT.hasLocalQualifiers());
  // TODO(zarko): ObjC tycons; embedded C tycons (address spaces).
  if (auto ID = BuildNodeSetForType(QT.getTypePtr())) {
    // Don't look down into type aliases. We'll have hit those during the
    // BuildNodeIdForType call above.
    // TODO(zarko): also add canonical edges (what do we call the edges?
    // 'expanded' seems reasonable).
    //   using ConstInt = const int;
    //   using CVInt1 = volatile ConstInt;
    if (QT.isLocalConstQualified()) {
      ID = ApplyBuiltinTypeConstructor("const", *ID);
    }
    if (QT.isLocalRestrictQualified()) {
      ID = ApplyBuiltinTypeConstructor("restrict", *ID);
    }
    if (QT.isLocalVolatileQualified()) {
      ID = ApplyBuiltinTypeConstructor("volatile", *ID);
    }
    return ID;
  }
  return NodeSet::Empty();
}

NodeSet IndexerASTVisitor::BuildNodeSetForTypeInternal(const clang::Type& T) {
  // There aren't too many types in C++, as it turns out. See
  // clang/AST/TypeNodes.def.
#define UNSUPPORTED_CLANG_TYPE(t)               \
  case clang::Type::t:                          \
    if (IgnoreUnimplemented) {                  \
      return NodeSet::Empty();                  \
    } else {                                    \
      LOG(FATAL) << "Type::" #t " unsupported"; \
    }                                           \
    break

#define DELEGATE_TYPE(t) \
  case clang::Type::t:   \
    return BuildNodeSetFor##t(clang::cast<t##Type>(T));
  // We only care about leaves in the type hierarchy (eg, we shouldn't match
  // on Reference, but instead on LValueReference or RValueReference).
  switch (T.getTypeClass()) {
    DELEGATE_TYPE(Builtin);           // Leaf.
    DELEGATE_TYPE(Enum);              // Leaf.
    DELEGATE_TYPE(TemplateTypeParm);  // Leaf.
    DELEGATE_TYPE(Record);            // Leaf.
    DELEGATE_TYPE(ObjCInterface);     // Leaf.
    DELEGATE_TYPE(ObjCObjectPointer);
    DELEGATE_TYPE(ObjCTypeParam);
    DELEGATE_TYPE(ObjCObject);
    DELEGATE_TYPE(Pointer);
    DELEGATE_TYPE(MemberPointer);
    DELEGATE_TYPE(LValueReference);
    DELEGATE_TYPE(RValueReference);
    DELEGATE_TYPE(Auto);
    DELEGATE_TYPE(DeducedTemplateSpecialization);
    DELEGATE_TYPE(ConstantArray);
    DELEGATE_TYPE(IncompleteArray);
    DELEGATE_TYPE(DependentSizedArray);
    DELEGATE_TYPE(FunctionProto);
    DELEGATE_TYPE(FunctionNoProto);
    DELEGATE_TYPE(Paren);
    DELEGATE_TYPE(Typedef);
    DELEGATE_TYPE(Decltype);
    DELEGATE_TYPE(Elaborated);
    // "Within an instantiated template, all template type parameters have
    // been replaced with these. They are used solely to record that a type
    // was originally written as a template type parameter; therefore they are
    // never canonical."
    DELEGATE_TYPE(SubstTemplateTypeParm);
    DELEGATE_TYPE(InjectedClassName);
    DELEGATE_TYPE(DependentName);
    DELEGATE_TYPE(PackExpansion);
    DELEGATE_TYPE(BlockPointer);
    DELEGATE_TYPE(TemplateSpecialization);
    DELEGATE_TYPE(Attributed);
    DELEGATE_TYPE(DependentAddressSpace);
    DELEGATE_TYPE(ExtInt);
    DELEGATE_TYPE(DependentExtInt);
    UNSUPPORTED_CLANG_TYPE(DependentTemplateSpecialization);
    UNSUPPORTED_CLANG_TYPE(Complex);
    UNSUPPORTED_CLANG_TYPE(VariableArray);
    UNSUPPORTED_CLANG_TYPE(DependentSizedExtVector);
    UNSUPPORTED_CLANG_TYPE(Vector);
    UNSUPPORTED_CLANG_TYPE(ExtVector);
    UNSUPPORTED_CLANG_TYPE(Adjusted);
    UNSUPPORTED_CLANG_TYPE(Decayed);
    UNSUPPORTED_CLANG_TYPE(TypeOfExpr);
    UNSUPPORTED_CLANG_TYPE(TypeOf);
    UNSUPPORTED_CLANG_TYPE(UnresolvedUsing);
    UNSUPPORTED_CLANG_TYPE(UnaryTransform);
    // "When a pack expansion in the source code contains multiple parameter
    // packs and those parameter packs correspond to different levels of
    // template parameter lists, this type node is used to represent a
    // template type parameter pack from an outer level, which has already had
    // its argument pack substituted but that still lives within a pack
    // expansion that itself could not be instantiated. When actually
    // performing a substitution into that pack expansion (e.g., when all
    // template parameters have corresponding arguments), this type will be
    // replaced with the SubstTemplateTypeParmType at the current pack
    // substitution index."
    UNSUPPORTED_CLANG_TYPE(SubstTemplateTypeParmPack);
    UNSUPPORTED_CLANG_TYPE(Atomic);
    UNSUPPORTED_CLANG_TYPE(Pipe);
    UNSUPPORTED_CLANG_TYPE(DependentVector);
    UNSUPPORTED_CLANG_TYPE(MacroQualified);
    UNSUPPORTED_CLANG_TYPE(ConstantMatrix);
    UNSUPPORTED_CLANG_TYPE(DependentSizedMatrix);
  }
#undef UNSUPPORTED_CLANG_TYPE
#undef DELEGATE_TYPE
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdForObjCProtocols(const clang::ObjCObjectType& T) {
  if (T.getInterface()) {
    if (auto BaseId = BuildNodeIdForType(T.getBaseType())) {
      return BuildNodeIdForObjCProtocols(
          BuildNodeIdsForObjCProtocols(*BaseId, T));
    } else {
      return absl::nullopt;
    }
  } else {
    return BuildNodeIdForObjCProtocols(BuildNodeIdsForObjCProtocols(T));
  }
}

GraphObserver::NodeId IndexerASTVisitor::BuildNodeIdForObjCProtocols(
    absl::Span<const GraphObserver::NodeId> ProtocolIds) {
  if (ProtocolIds.empty()) {
    return Observer.getNodeIdForBuiltinType("id");
  } else if (ProtocolIds.size() == 1) {
    // We have something like id<P1>. This is a special case of the following
    // code that handles id<P1, P2> because *this* case can skip the
    // intermediate union tapp.
    return ProtocolIds.front();
  }
  // Create/find the Union node.
  auto UnionTApp = Observer.getNodeIdForBuiltinType("TypeUnion");
  return Observer.recordTappNode(UnionTApp, ProtocolIds);
}

// Base case where we don't have a separate BaseType to contend with
// (BaseType is just an `id` node).
std::vector<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdsForObjCProtocols(const ObjCObjectType& T) {
  // Use a multimap since it is sorted by key and we want our nodes sorted by
  // their (uncompressed) name. We want the items sorted by the original class
  // name because the user should be able to write down a union type
  // for the verifier and they can only do that if they know the order in
  // which the types will be passed as parameters.
  std::multimap<std::string, GraphObserver::NodeId> ProtocolNodes;
  for (ObjCProtocolDecl* P : T.getProtocols()) {
    ProtocolNodes.insert({P->getNameAsString(), BuildNodeIdForDecl(P)});
  }
  std::vector<GraphObserver::NodeId> ProtocolIds;
  ProtocolIds.reserve(ProtocolNodes.size());
  for (const auto& PN : ProtocolNodes) {
    ProtocolIds.push_back(PN.second);
  }
  return ProtocolIds;
}

std::vector<GraphObserver::NodeId>
IndexerASTVisitor::BuildNodeIdsForObjCProtocols(GraphObserver::NodeId BaseType,
                                                const ObjCObjectType& T) {
  auto ProtocolIds = BuildNodeIdsForObjCProtocols(T);
  ProtocolIds.insert(ProtocolIds.begin(), BaseType);
  return ProtocolIds;
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
    const clang::ObjCPropertyImplDecl* Decl) {
  return true;
}

// This is like a typedef at a high level. It says that class A can be used
// instead of class B if class B does not exist.
bool IndexerASTVisitor::VisitObjCCompatibleAliasDecl(
    const clang::ObjCCompatibleAliasDecl* Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  // TODO(salguarnieri) Find a better way to parse the tokens to account for
  // macros with parameters.

  // Ugliness to get the ranges for the tokens in this decl since clang does
  // not give them to us. We expect something of the form:
  // @compatibility_alias AliasName OriginalClassName
  // Note that this does not work in the presence of macros with parameters.
  const auto AliasRange = RangeForNameOfDeclaration(Decl);

  // Record a ref to the original type
  if (const auto OrigClass = clang::Lexer::findNextToken(
          AliasRange.getEnd(), *Observer.getSourceManager(),
          *Observer.getLangOptions())) {
    if (const auto& ERCC = ExplicitRangeInCurrentContext(clang::SourceRange(
            OrigClass->getLocation(), OrigClass->getEndLoc()))) {
      const auto& ID = BuildNodeIdForDecl(Decl->getClassInterface());
      Observer.recordDeclUseLocation(ERCC.value(), ID,
                                     GraphObserver::Claimability::Claimable,
                                     IsImplicit(ERCC.value()));
    }
  }

  // Record the type alias
  const auto& OriginalInterface = Decl->getClassInterface();
  GraphObserver::NameId AliasID(BuildNameIdForDecl(Decl));
  auto AliasedTypeID(BuildNodeIdForDecl(OriginalInterface));
  auto AliasNode = Observer.recordTypeAliasNode(
      AliasID, AliasedTypeID, AliasedTypeID,
      Marks.GenerateMarkedSource(
          Observer.nodeIdForTypeAliasNode(AliasID, AliasedTypeID)));
  AssignUSR(AliasID, AliasedTypeID, Decl);

  // Record the definition of this type alias
  MaybeRecordDefinitionRange(ExplicitRangeInCurrentContext(AliasRange),
                             AliasNode, absl::nullopt);
  AddChildOfEdgeToDeclContext(Decl, AliasNode);

  return true;
}

bool IndexerASTVisitor::VisitObjCImplementationDecl(
    const clang::ObjCImplementationDecl* ImplDecl) {
  auto Marks = MarkedSources.Generate(ImplDecl);
  SourceRange NameRange = RangeForNameOfDeclaration(ImplDecl);
  auto DeclNode = BuildNodeIdForDecl(ImplDecl);

  MaybeRecordDefinitionRange(
      RangeInCurrentContext(ImplDecl->isImplicit(), DeclNode, NameRange),
      DeclNode, absl::nullopt);
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
            NameRangeInContext.value(), TargetDecl,
            InterfaceFile == DeclFile
                ? GraphObserver::Specificity::UniquelyCompletes
                : GraphObserver::Specificity::Completes,
            DeclNode);
      }
      RecordCompletesForRedecls(ImplDecl, NameRange, DeclNode);
    }
    ConnectToSuperClassAndProtocols(DeclNode, Interface);
  } else {
    LogErrorWithASTDump("Missing class interface", ImplDecl);
  }
  AssignUSR(DeclNode, ImplDecl);
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
    const clang::ObjCCategoryImplDecl* ImplDecl) {
  auto Marks = MarkedSources.Generate(ImplDecl);
  SourceRange NameRange = NormalizeRange(ImplDecl->getCategoryNameLoc());
  auto ImplDeclNode = BuildNodeIdForDecl(ImplDecl);
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(ImplDecl->isImplicit(), ImplDeclNode, NameRange),
      ImplDeclNode, absl::nullopt);
  AddChildOfEdgeToDeclContext(ImplDecl, ImplDeclNode);

  FileID ImplDeclFile =
      Observer.getSourceManager()->getFileID(ImplDecl->getCategoryNameLoc());

  AssignUSR(ImplDeclNode, ImplDecl);
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
            NameRangeInContext.value(), DeclNode,
            DeclFile == ImplDeclFile
                ? GraphObserver::Specificity::UniquelyCompletes
                : GraphObserver::Specificity::Completes,
            ImplDeclNode);
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
    auto Range = NormalizeRange(ImplDecl->getCategoryNameLoc());
    if (auto RCC = ExplicitRangeInCurrentContext(Range)) {
      auto ID = BuildNodeIdForDecl(CategoryDecl);
      Observer.recordDeclUseLocation(RCC.value(), ID,
                                     GraphObserver::Claimability::Unclaimable,
                                     IsImplicit(RCC.value()));
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
    const SourceRange& IFaceNameRange = NormalizeRange(ImplDecl->getLocation());
    if (auto RCC = ExplicitRangeInCurrentContext(IFaceNameRange)) {
      Observer.recordDeclUseLocation(RCC.value(), ClassInterfaceNode,
                                     GraphObserver::Claimability::Claimable,
                                     IsImplicit(RCC.value()));
    }
  } else {
    LogErrorWithASTDump("Missing category impl class interface", ImplDecl);
  }

  return true;
}

void IndexerASTVisitor::ConnectToSuperClassAndProtocols(
    const GraphObserver::NodeId BodyDeclNode,
    const clang::ObjCInterfaceDecl* IFace) {
  if (IsObjCForwardDecl(IFace)) {
    // This is a forward declaration, so it won't have superclass or protocol
    // info
    return;
  }

  if (const auto* SCTSI = IFace->getSuperClassTInfo()) {
    if (auto SCType = BuildNodeIdForType(SCTSI->getTypeLoc())) {
      Observer.recordExtendsEdge(BodyDeclNode, SCType.value(),
                                 false /* isVirtual */,
                                 clang::AccessSpecifier::AS_none);
    }
  }
  // Draw a ref edge from the superclass usage in the interface declaration to
  // the superclass declaration.
  if (auto SC = IFace->getSuperClass()) {
    auto SuperRange = NormalizeRange(IFace->getSuperClassLoc());
    if (auto SCRCC = ExplicitRangeInCurrentContext(SuperRange)) {
      auto SCID = BuildNodeIdForDecl(SC);
      Observer.recordDeclUseLocation(SCRCC.value(), SCID,
                                     GraphObserver::Claimability::Unclaimable,
                                     IsImplicit(SCRCC.value()));
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
    auto Range = NormalizeRange(*PLocIt);
    if (auto ERCC = ExplicitRangeInCurrentContext(Range)) {
      auto PID = BuildNodeIdForDecl(*PIt);
      Observer.recordDeclUseLocation(ERCC.value(), PID,
                                     GraphObserver::Claimability::Unclaimable,
                                     IsImplicit(ERCC.value()));
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
    const clang::ObjCInterfaceDecl* Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  GraphObserver::NodeId BodyDeclNode(Observer.getDefaultClaimToken(), "");
  GraphObserver::NodeId DeclNode(Observer.getDefaultClaimToken(), "");

  // If we have type arguments, treat this as a generic type and indirect
  // through an abs node.
  if (const auto* TPL = Decl->getTypeParamList()) {
    BodyDeclNode = BuildNodeIdForDecl(Decl, 0);
    DeclNode = RecordGenericClass(Decl, TPL, BodyDeclNode);
  } else {
    BodyDeclNode = BuildNodeIdForDecl(Decl);
    DeclNode = BodyDeclNode;
  }

  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode,
      absl::nullopt);

  AddChildOfEdgeToDeclContext(Decl, DeclNode);

  auto Completeness = IsObjCForwardDecl(Decl)
                          ? GraphObserver::Completeness::Incomplete
                          : GraphObserver::Completeness::Complete;
  AssignUSR(BodyDeclNode, Decl);
  Observer.recordRecordNode(BodyDeclNode, GraphObserver::RecordKind::Class,
                            Completeness, absl::nullopt);
  Observer.recordMarkedSource(DeclNode, Marks.GenerateMarkedSource(DeclNode));
  RecordCompletesForRedecls(Decl, NameRange, BodyDeclNode);
  ConnectToSuperClassAndProtocols(BodyDeclNode, Decl);
  return true;
}

// Categories and Classes are different things. You either have an
// ObjCInterfaceDecl *OR* a ObjCCategoryDecl.
bool IndexerASTVisitor::VisitObjCCategoryDecl(
    const clang::ObjCCategoryDecl* Decl) {
  // Use the category name as our name range. If this is an extension and has
  // no category name, use the name range for the interface decl.
  auto Marks = MarkedSources.Generate(Decl);
  SourceRange NameRange;
  if (Decl->IsClassExtension()) {
    NameRange = RangeForNameOfDeclaration(Decl);
  } else {
    NameRange = NormalizeRange(Decl->getCategoryNameLoc());
  }

  auto DeclNode = BuildNodeIdForDecl(Decl);
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode,
      absl::nullopt);
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  AssignUSR(DeclNode, Decl);
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
    const SourceRange& IFaceNameRange = NormalizeRange(Decl->getLocation());
    if (auto RCC = ExplicitRangeInCurrentContext(IFaceNameRange)) {
      Observer.recordDeclUseLocation(RCC.value(), ClassInterfaceNode,
                                     GraphObserver::Claimability::Claimable,
                                     IsImplicit(RCC.value()));
    }
  } else {
    LogErrorWithASTDump("Missing category decl class interface", Decl);
  }

  return true;
}

// This method is not inlined because it is important that we do the same
// thing for category declarations and category implementations.
void IndexerASTVisitor::ConnectCategoryToBaseClass(
    const GraphObserver::NodeId& DeclNode, const ObjCInterfaceDecl* IFace) {
  auto ClassTypeId = BuildNodeIdForDecl(IFace);
  Observer.recordCategoryExtendsEdge(DeclNode, ClassTypeId);
}

void IndexerASTVisitor::RecordCompletesForRedecls(
    const Decl* Decl, const SourceRange& NameRange,
    const GraphObserver::NodeId& DeclNode) {
  // Don't draw completion edges if this is a forward declared class in
  // Objective-C because forward declarations don't complete anything.
  if (const auto* I = dyn_cast<clang::ObjCInterfaceDecl>(Decl)) {
    if (IsObjCForwardDecl(I)) {
      return;
    }
  }

  FileID DeclFile = Observer.getSourceManager()->getFileID(Decl->getLocation());
  if (auto NameRangeInContext =
          RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange)) {
    for (const auto* NextDecl : Decl->redecls()) {
      // It's not useful to draw completion edges to implicit forward
      // declarations, nor is it useful to declare that a definition completes
      // itself.
      if (NextDecl != Decl && !NextDecl->isImplicit()) {
        FileID NextDeclFile =
            Observer.getSourceManager()->getFileID(NextDecl->getLocation());
        GraphObserver::NodeId TargetDecl = BuildNodeIdForDecl(NextDecl);
        Observer.recordCompletionRange(
            NameRangeInContext.value(), TargetDecl,
            NextDeclFile == DeclFile
                ? GraphObserver::Specificity::UniquelyCompletes
                : GraphObserver::Specificity::Completes,
            DeclNode);
      }
    }
  }
}

bool IndexerASTVisitor::VisitObjCProtocolDecl(
    const clang::ObjCProtocolDecl* Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  auto DeclNode = BuildNodeIdForDecl(Decl);

  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode,
      absl::nullopt);
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  Observer.recordInterfaceNode(DeclNode, Marks.GenerateMarkedSource(DeclNode));
  ConnectToProtocols(DeclNode, Decl->protocol_loc_begin(),
                     Decl->protocol_loc_end(), Decl->protocol_begin(),
                     Decl->protocol_end());
  return true;
}

bool IndexerASTVisitor::VisitObjCMethodDecl(const clang::ObjCMethodDecl* Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  auto Node = BuildNodeIdForDecl(Decl);
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  // TODO(salguarnieri) Use something more honest for the name in the marked
  // source. The "name" of an Objective-C method is not a single range in the
  // source code. Ex: -(void) myFunc:(int)data except:(int)moreData should
  // have the name "myFunc:except:".
  Marks.set_name_range(NameRange);
  Marks.set_marked_source_end(Decl->getSourceRange().getEnd());
  auto NameRangeInContext(
      RangeInCurrentContext(Decl->isImplicit(), Node, NameRange));
  MaybeRecordDefinitionRange(NameRangeInContext, Node, absl::nullopt);
  bool IsFunctionDefinition = Decl->isThisDeclarationADefinition();
  unsigned ParamNumber = 0;
  for (const auto* Param : Decl->parameters()) {
    ConnectParam(Decl, Node, IsFunctionDefinition, ParamNumber++, Param, false);
  }

  absl::optional<GraphObserver::NodeId> FunctionType =
      CreateObjCMethodTypeNode(Decl);

  if (FunctionType) {
    Observer.recordTypeEdge(Node, FunctionType.value());
  }

  // Record overrides edges
  SmallVector<const ObjCMethodDecl*, 4> overrides;
  Decl->getOverriddenMethods(overrides);
  for (const auto& O : overrides) {
    Observer.recordOverridesEdge(
        Node, absl::GetFlag(FLAGS_experimental_alias_template_instantiations)
                  ? BuildNodeIdForRefToDecl(O)
                  : BuildNodeIdForDecl(O));
  }
  if (!overrides.empty()) {
    MapOverrideRoots(Decl, [&](const ObjCMethodDecl* R) {
      Observer.recordOverridesRootEdge(
          Node, absl::GetFlag(FLAGS_experimental_alias_template_instantiations)
                    ? BuildNodeIdForRefToDecl(R)
                    : BuildNodeIdForDecl(R));
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
    // When we are visiting the definition for a method decl from an
    // extension, we don't have a way to get back to the declaration. The true
    // method decl does not appear in the list of redecls and it is not
    // returned by getCanonicalDecl. Furthermore, there is no way to get
    // access to the extension declaration. Since class implementations do not
    // mention extensions, there is nothing for us to use.
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
        ObjCImplementationDecl* ClassImpl =
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
                  ImplNameRangeInContext.value(), BuildNodeIdForDecl(Decl),
                  DeclFile == ImplFile
                      ? GraphObserver::Specificity::UniquelyCompletes
                      : GraphObserver::Specificity::Completes,
                  Node);
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
    const GraphObserver::Range& DeclNameRangeInContext =
        NameRangeInContext.value();

    // If we want to draw an edge from method impl to property we can modify
    // the following code:
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
                                 : GraphObserver::Specificity::Completes,
          Node);
    }

    // Connect all other redecls to this definition with a completion edge.
    for (const auto* NextDecl : Decl->redecls()) {
      // Do not draw self-completion edges and do not draw an edge for the
      // canonical decl again.
      if (NextDecl != Decl && NextDecl != CD) {
        FileID RedeclFile =
            Observer.getSourceManager()->getFileID(NextDecl->getLocation());
        Observer.recordCompletionRange(
            DeclNameRangeInContext, BuildNodeIdForDecl(NextDecl),
            RedeclFile == DefnFile
                ? GraphObserver::Specificity::UniquelyCompletes
                : GraphObserver::Specificity::Completes,
            Node);
      }
    }
  }
  Observer.recordFunctionNode(Node, GraphObserver::Completeness::Definition,
                              Subkind, Marks.GenerateMarkedSource(Node));
  return true;
}

// TODO(salguarnieri) Do we need to record a use for the parameter type?
void IndexerASTVisitor::ConnectParam(const Decl* Decl,
                                     const GraphObserver::NodeId& FuncNode,
                                     bool IsFunctionDefinition,
                                     const unsigned int ParamNumber,
                                     const ParmVarDecl* Param,
                                     bool DeclIsImplicit) {
  auto Marks = MarkedSources.Generate(Param);
  GraphObserver::NodeId VarNodeId(BuildNodeIdForDecl(Param));
  SourceRange Range = RangeForNameOfDeclaration(Param);
  Marks.set_name_range(Range);
  Marks.set_implicit(Job->UnderneathImplicitTemplateInstantiation ||
                     DeclIsImplicit);
  Marks.set_marked_source_end(NormalizeRange(Param->getSourceRange()).getEnd());
  Observer.recordVariableNode(VarNodeId,
                              IsFunctionDefinition
                                  ? GraphObserver::Completeness::Definition
                                  : GraphObserver::Completeness::Incomplete,
                              GraphObserver::VariableSubkind::None,
                              Marks.GenerateMarkedSource(VarNodeId));
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Param->isImplicit() || Decl->isImplicit(),
                            VarNodeId, Range),
      VarNodeId, BuildNodeIdForDefnOfDecl(Param));
  Observer.recordParamEdge(FuncNode, ParamNumber, VarNodeId);
  absl::optional<GraphObserver::NodeId> ParamType;
  if (auto* TSI = Param->getTypeSourceInfo()) {
    ParamType = BuildNodeIdForType(TSI->getTypeLoc());
  } else {
    CHECK(!Param->getType().isNull());
    ParamType = BuildNodeIdForType(Param->getType());
  }
  if (ParamType) {
    Observer.recordTypeEdge(VarNodeId, ParamType.value());
  }
}

// TODO(salguarnieri) Think about merging with VisitFieldDecl
bool IndexerASTVisitor::VisitObjCPropertyDecl(
    const clang::ObjCPropertyDecl* Decl) {
  auto Marks = MarkedSources.Generate(Decl);
  GraphObserver::NodeId DeclNode(BuildNodeIdForDecl(Decl));
  SourceRange NameRange = RangeForNameOfDeclaration(Decl);
  Marks.set_name_range(NameRange);
  Marks.set_marked_source_end(Decl->getSourceRange().getEnd());
  MaybeRecordDefinitionRange(
      RangeInCurrentContext(Decl->isImplicit(), DeclNode, NameRange), DeclNode,
      absl::nullopt);
  // TODO(zarko): Record completeness data. This is relevant for static
  // fields, which may be declared along with a complete class definition but
  // later defined in a separate translation unit.
  Observer.recordVariableNode(
      DeclNode, GraphObserver::Completeness::Definition,
      // TODO(salguarnieri) Think about making a new subkind for properties.
      GraphObserver::VariableSubkind::Field,
      Marks.GenerateMarkedSource(DeclNode));
  AssignUSR(DeclNode, Decl);
  if (auto TyNodeId = BuildNodeIdForType(Decl->getType())) {
    Observer.recordTypeEdge(DeclNode, *TyNodeId);
  }
  AddChildOfEdgeToDeclContext(Decl, DeclNode);
  return true;
}

bool IndexerASTVisitor::VisitObjCIvarRefExpr(
    const clang::ObjCIvarRefExpr* IRE) {
  return VisitDeclRefOrIvarRefExpr(IRE, IRE->getDecl(), IRE->getLocation());
}

bool IndexerASTVisitor::VisitObjCMessageExpr(
    const clang::ObjCMessageExpr* Expr) {
  if (auto RCC = ExpandedRangeInCurrentContext(Expr->getSourceRange())) {
    // This does not take dynamic dispatch into account when looking for the
    // method definition.
    if (const auto* Callee = FindMethodDefn(Expr->getMethodDecl(),
                                            Expr->getReceiverInterface())) {
      const GraphObserver::NodeId& DeclId = BuildNodeIdForRefToDecl(Callee);
      RecordCallEdges(RCC.value(), DeclId);

      // We only record a ref if we have successfully recorded a ref/call.
      // If we don't have any selectors, just use the same span as the
      // ref/call.
      if (Expr->getNumSelectorLocs() == 0) {
        Observer.recordDeclUseLocation(RCC.value(), DeclId,
                                       GraphObserver::Claimability::Unclaimable,
                                       IsImplicit(RCC.value()));
      } else {
        // TODO Record multiple ranges, one for each selector.
        // For now, just record the range for the first selector. This should
        // make it easier for frontends to make use of this data.
        const SourceLocation& Loc = Expr->getSelectorLoc(0);
        if (Loc.isValid() && Loc.isFileID()) {
          SourceRange range = NormalizeRange(Loc);
          if (auto R = ExplicitRangeInCurrentContext(range)) {
            Observer.recordDeclUseLocation(
                R.value(), DeclId, GraphObserver::Claimability::Unclaimable,
                IsImplicit(R.value()));
          }
        }
      }
    }
  }
  return true;
}

const ObjCMethodDecl* IndexerASTVisitor::FindMethodDefn(
    const ObjCMethodDecl* MD, const ObjCInterfaceDecl* I) {
  if (MD == nullptr || I == nullptr || MD->isThisDeclarationADefinition()) {
    return MD;
  }
  // If we can, look in the implementation, otherwise we look in the
  // interface.
  const ObjCContainerDecl* CD = I->getImplementation();
  if (CD == nullptr) {
    CD = I;
  }
  if (const auto* MI =
          CD->getMethod(MD->getSelector(), MD->isInstanceMethod())) {
    return MI;
  }
  return MD;
}

bool IndexerASTVisitor::VisitObjCPropertyRefExpr(
    const clang::ObjCPropertyRefExpr* Expr) {
  // Both implicit and explicit properties will provide a backing method decl.
  ObjCMethodDecl* MD = nullptr;
  ObjCPropertyDecl* PD = nullptr;
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
    SourceRange SR = NormalizeRange(SL);
    auto StmtId = BuildNodeIdForImplicitStmt(Expr);
    if (auto RCC = RangeInCurrentContext(StmtId, SR)) {
      // Record the "field" access if this has an explicit property.
      if (PD != nullptr) {
        GraphObserver::NodeId DeclId = BuildNodeIdForDecl(PD);
        Observer.recordDeclUseLocation(RCC.value(), DeclId,
                                       GraphObserver::Claimability::Unclaimable,
                                       IsImplicit(RCC.value()));
        for (const auto& S : Supports) {
          S->InspectDeclRef(*this, SL, RCC.value(), DeclId, PD);
        }
      }

      // Record the method call.

      // TODO(salguarnieri) Try to prevent the call edge unless the user has
      // defined a custom setter or getter. This is a non-trivial
      // because the user may provide a custom implementation using the
      // default name. Otherwise, we could just test the method decl
      // location.
      if (MD != nullptr) {
        RecordCallEdges(RCC.value(), BuildNodeIdForRefToDecl(MD));
      }
    }
  }

  return true;
}

absl::optional<GraphObserver::NodeId>
IndexerASTVisitor::CreateObjCMethodTypeNode(const clang::ObjCMethodDecl* MD) {
  std::vector<GraphObserver::NodeId> NodeIds;
  // If we are in an implicit method (for example: property access), we may
  // not get return type source information and we will have to rely on the
  // QualType provided by getReturnType.
  auto R = MD->getReturnTypeSourceInfo();
  auto ReturnType = R ? BuildNodeIdForType(R->getTypeLoc())
                      : BuildNodeIdForType(MD->getReturnType());
  if (!ReturnType) {
    return ReturnType;
  }

  NodeIds.push_back(ReturnType.value());
  for (auto PVD : MD->parameters()) {
    if (PVD) {
      auto TSI = PVD->getTypeSourceInfo();
      auto ParamType = TSI ? BuildNodeIdForType(TSI->getTypeLoc())
                           : BuildNodeIdForType(PVD->getType());
      if (!ParamType) {
        return ParamType;
      }
      NodeIds.push_back(ParamType.value());
    }
  }

  // TODO(salguarnieri) Make this a constant somewhere
  const char* Tycon = "fn";
  return Observer.recordTappNode(Observer.getNodeIdForBuiltinType(Tycon),
                                 NodeIds);
}

void IndexerASTVisitor::LogErrorWithASTDump(absl::string_view msg,
                                            const clang::Decl* Decl) const {
  std::string s;
  llvm::raw_string_ostream ss(s);
  Decl->dump(ss);
  LOG(ERROR) << msg << " :" << std::endl << s;
}

void IndexerASTVisitor::LogErrorWithASTDump(absl::string_view msg,
                                            const clang::Expr* Expr) const {
  std::string s;
  llvm::raw_string_ostream ss(s);
  Expr->dump(ss, Context);
  LOG(ERROR) << msg << " :" << std::endl << s;
}

}  // namespace kythe
