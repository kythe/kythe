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
// Defines AST visitors and consumers used by the indexer tool.

#ifndef KYTHE_CXX_INDEXER_CXX_INDEXER_AST_HOOKS_H_
#define KYTHE_CXX_INDEXER_CXX_INDEXER_AST_HOOKS_H_

#include <memory>
#include <unordered_map>

#include "glog/logging.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTTypeTraits.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Index/USRGeneration.h"
#include "clang/Sema/SemaConsumer.h"
#include "clang/Sema/Template.h"

#include "GraphObserver.h"
#include "IndexerLibrarySupport.h"

namespace kythe {

/// \brief None() may be used to construct an empty `MaybeFew<T>`.
struct None {};

/// \brief A class containing zero or more instances of copyable `T`. When there
/// are more than zero elements, the first is distinguished as the `primary`.
///
/// This class is primarily used to hold type NodeIds, as the process for
/// deriving one may fail (if the type is unsupported) or may return more than
/// one possible ID (if the type is noncanonical).
template <typename T> class MaybeFew {
public:
  /// \brief Constructs a new `MaybeFew` holding zero or more elements.
  /// If there are more than zero elements, the first one provided is primary.
  template <typename... Ts>
  MaybeFew(Ts &&... Init) : Content({std::forward<T>(Init)...}) {}

  /// \brief Constructs an empty `MaybeFew`.
  ///
  /// This is meant to be used in contexts where you would like the compiler
  /// to deduce the MaybeFew's template parameter; for example, in a function
  /// returning `MaybeFew<S>`, you may `return None()` without repeating `S`.
  MaybeFew(None) {}

  /// \brief Returns true iff there are elements.
  explicit operator bool() const { return !Content.empty(); }

  /// \brief Returns the primary element (the first one provided during
  /// construction).
  const T &primary() const {
    CHECK(!Content.empty());
    return Content[0];
  }

  /// \brief Take all of the elements previously stored. The `MaybeFew` remains
  /// in an indeterminate state afterward.
  std::vector<T> &&takeAll() { return std::move(Content); }

  /// \brief Returns a reference to the internal vector holding the elements.
  /// If the vector is non-empty, the first element is primary.
  const std::vector<T> &all() { return Content; }

  /// \brief Creates a new `MaybeFew` by applying `Fn` to each element of this
  /// one. Order is preserved (st `A.Map(Fn).all()[i] = Fn(A.all()[i])`); also,
  /// `A.Map(Fn).all().size() == A.all().size()`.
  template <typename S>
  MaybeFew<S> Map(const std::function<S(const T &)> &Fn) const {
    MaybeFew<S> Result;
    Result.Content.reserve(Content.size());
    for (const auto &E : Content) {
      Result.Content.push_back(Fn(E));
    }
    return Result;
  }

  /// \brief Apply `Fn` to each stored element starting with the primary.
  void Iter(const std::function<void(const T &)> &Fn) const {
    for (const auto &E : Content) {
      Fn(E);
    }
  }

private:
  /// The elements stored in this value, where `Content[0]` is the primary
  /// element (if any elements exist at all).
  std::vector<T> Content;
};

/// \brief Constructs a `MaybeFew<T>` given a single `T`.
template <typename T> MaybeFew<T> Some(T &&t) {
  return MaybeFew<T>(std::forward<T>(t));
}

/// For a given node in the AST, this class keeps track of the node's
/// parent (along some path from the AST root) and an integer index for that
/// node in some arbitrary but consistent order defined by the parent.
struct IndexedParent {
  /// \brief The parent DynTypedNode associated with some key.
  clang::ast_type_traits::DynTypedNode Parent;
  /// \brief The index at which some associated key appears in `Parent`.
  size_t Index;
};

inline bool operator==(const IndexedParent &L, const IndexedParent &R) {
  // We compare IndexedParents for deduplicating memoizable DynTypedNodes
  // below; semantically, this means that we keep the first child index
  // we saw when following every path through a particular memoizable
  // IndexedParent.
  return L.Parent == R.Parent;
}

inline bool operator!=(const IndexedParent &L, const IndexedParent &R) {
  return !(L == R);
}

typedef llvm::SmallVector<IndexedParent, 2> IndexedParentVector;

typedef llvm::DenseMap<
    const void *, llvm::PointerUnion<IndexedParent *, IndexedParentVector *>>
    IndexedParentMap;

/// FIXME: Currently only builds up the map using \c Stmt and \c Decl nodes.
/// TODO(zarko): Is this necessary to change for naming?
class IndexedParentASTVisitor
    : public clang::RecursiveASTVisitor<IndexedParentASTVisitor> {
public:
  /// \brief Builds and returns the translation unit's indexed parent map.
  ///
  /// The caller takes ownership of the returned `IndexedParentMap`.
  static std::unique_ptr<IndexedParentMap>
  buildMap(clang::TranslationUnitDecl &TU) {
    std::unique_ptr<IndexedParentMap> ParentMap(new IndexedParentMap);
    IndexedParentASTVisitor Visitor(ParentMap.get());
    Visitor.TraverseDecl(&TU);
    return ParentMap;
  }

private:
  typedef RecursiveASTVisitor<IndexedParentASTVisitor> VisitorBase;

  explicit IndexedParentASTVisitor(IndexedParentMap *Parents)
      : Parents(Parents) {}

  bool shouldVisitTemplateInstantiations() const { return true; }
  bool shouldVisitImplicitCode() const { return true; }
  // Disables data recursion. We intercept Traverse* methods in the RAV, which
  // are not triggered during data recursion.
  bool shouldUseDataRecursionFor(clang::Stmt *S) const { return false; }

  // Traverse an arbitrary AST node type and record the node used to get to
  // it as that node's parent. `T` is the type of the node and
  // `BaseTraverseFn` is the type of a function (or other value with
  // an operator()) that invokes the base RecursiveASTVisitor traversal logic
  // on values of type `T*` and returns a boolean traversal result.
  template <typename T, typename BaseTraverseFn>
  bool TraverseNode(T *Node, BaseTraverseFn traverse) {
    if (!Node)
      return true;
    if (!ParentStack.empty()) {
      // FIXME: Currently we add the same parent multiple times, but only
      // when no memoization data is available for the type.
      // For example when we visit all subexpressions of template
      // instantiations; this is suboptimal, but benign: the only way to
      // visit those is with hasAncestor / hasParent, and those do not create
      // new matches.
      // The plan is to enable DynTypedNode to be storable in a map or hash
      // map. The main problem there is to implement hash functions /
      // comparison operators for all types that DynTypedNode supports that
      // do not have pointer identity.
      auto &NodeOrVector = (*Parents)[Node];
      if (NodeOrVector.isNull()) {
        NodeOrVector = new IndexedParent(ParentStack.back());
      } else {
        if (NodeOrVector.template is<IndexedParent *>()) {
          auto *Node = NodeOrVector.template get<IndexedParent *>();
          auto *Vector = new IndexedParentVector(1, *Node);
          NodeOrVector = Vector;
          delete Node;
        }
        CHECK(NodeOrVector.template is<IndexedParentVector *>());

        auto *Vector = NodeOrVector.template get<IndexedParentVector *>();
        // Skip duplicates for types that have memoization data.
        // We must check that the type has memoization data before calling
        // std::find() because DynTypedNode::operator== can't compare all
        // types.
        bool Found = ParentStack.back().Parent.getMemoizationData() &&
                     std::find(Vector->begin(), Vector->end(),
                               ParentStack.back()) != Vector->end();
        if (!Found)
          Vector->push_back(ParentStack.back());
      }
    }
    ParentStack.push_back(
        {clang::ast_type_traits::DynTypedNode::create(*Node), 0});
    bool Result = traverse(Node);
    ParentStack.pop_back();
    if (!ParentStack.empty()) {
      ParentStack.back().Index++;
    }
    return Result;
  }

  bool TraverseDecl(clang::Decl *DeclNode) {
    return TraverseNode(DeclNode, [this](clang::Decl *Node) {
      return VisitorBase::TraverseDecl(Node);
    });
  }

  bool TraverseStmt(clang::Stmt *StmtNode) {
    return TraverseNode(StmtNode, [this](clang::Stmt *Node) {
      return VisitorBase::TraverseStmt(Node);
    });
  }

  IndexedParentMap *Parents;
  llvm::SmallVector<IndexedParent, 16> ParentStack;

  friend class RecursiveASTVisitor<IndexedParentASTVisitor>;
};

/// \brief Specifies whether uncommonly-used data should be dropped.
enum Verbosity : bool {
  Classic = true, ///< Emit all data.
  Lite = false    ///< Emit only common data.
};

/// \brief Specifies what the indexer should do if it encounters a case it
/// doesn't understand.
enum BehaviorOnUnimplemented : bool {
  Abort = false,  ///< Stop indexing and exit with an error.
  Continue = true ///< Continue indexing, possibly emitting less data.
};

/// \brief Specifies what the indexer should do with template instantiations.
enum BehaviorOnTemplates : bool {
  SkipInstantiations = false, ///< Don't visit template instantiations.
  VisitInstantiations = true  ///< Visit template instantiations.
};

/// \brief A byte range that links to some node.
struct MiniAnchor {
  size_t Begin;
  size_t End;
  GraphObserver::NodeId AnchoredTo;
};

/// Adds brackets to Text to define anchor locations (escaping existing ones)
/// and sorts Anchors such that the ith Anchor corresponds to the ith opening
/// bracket. Drops empty or negative-length spans.
void InsertAnchorMarks(std::string &Text, std::vector<MiniAnchor> &Anchors);

/// \brief An AST visitor that extracts information for a translation unit and
/// writes it to a `GraphObserver`.
class IndexerASTVisitor : public clang::RecursiveASTVisitor<IndexerASTVisitor> {
public:
  IndexerASTVisitor(clang::ASTContext &C, BehaviorOnUnimplemented B,
                    BehaviorOnTemplates T, Verbosity V,
                    const LibrarySupports &S, clang::Sema &Sema,
                    GraphObserver *GO = nullptr)
      : IgnoreUnimplemented(B), TemplateMode(T), Verbosity(V),
        Observer(GO ? *GO : NullObserver), Context(C), Supports(S), Sema(Sema) {
  }

  ~IndexerASTVisitor() { deleteAllParents(); }

  bool VisitDecl(const clang::Decl *Decl);
  bool VisitFieldDecl(const clang::FieldDecl *Decl);
  bool VisitVarDecl(const clang::VarDecl *Decl);
  bool VisitNamespaceDecl(const clang::NamespaceDecl *Decl);
  bool VisitDeclRefExpr(const clang::DeclRefExpr *DRE);
  bool VisitUnaryExprOrTypeTraitExpr(const clang::UnaryExprOrTypeTraitExpr *E);
  bool VisitCXXConstructExpr(const clang::CXXConstructExpr *E);
  bool VisitCXXDeleteExpr(const clang::CXXDeleteExpr *E);
  bool VisitCXXPseudoDestructorExpr(const clang::CXXPseudoDestructorExpr *E);
  bool
  VisitCXXUnresolvedConstructExpr(const clang::CXXUnresolvedConstructExpr *E);
  bool VisitCallExpr(const clang::CallExpr *Expr);
  bool VisitMemberExpr(const clang::MemberExpr *Expr);
  bool VisitCXXDependentScopeMemberExpr(
      const clang::CXXDependentScopeMemberExpr *Expr);
  bool VisitTypedefNameDecl(const clang::TypedefNameDecl *Decl);
  bool VisitRecordDecl(const clang::RecordDecl *Decl);
  bool VisitEnumDecl(const clang::EnumDecl *Decl);
  bool VisitEnumConstantDecl(const clang::EnumConstantDecl *Decl);
  bool VisitFunctionDecl(clang::FunctionDecl *Decl);
  bool TraverseDecl(clang::Decl *Decl);

  /// \brief For functions that support it, controls the emission of range
  /// information.
  enum class EmitRanges {
    No, ///< Don't emit range information.
    Yes ///< Emit range information when it's available.
  };

  /// \brief Builds a stable node ID for a compile-time expression.
  /// \param Expr The expression to represent.
  /// \param ER Whether to notify the `GraphObserver` about source text
  /// ranges for expressions.
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForExpr(const clang::Expr *Expr,
                                                     EmitRanges ER);

  /// \brief Builds a stable node ID for `Type`.
  /// \param Type The type that is being identified. If its location is valid
  /// and `ER` is `EmitRanges::Yes`, notifies the attached `GraphObserver` about
  /// the location of constituent elements.
  /// \param DType The deduced form of `Type`. (May be `Type.getTypePtr()`).
  /// \param ER whether to notify the `GraphObserver` about source text ranges
  /// for types.
  /// \return The Node ID for `Type`.
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForType(const clang::TypeLoc &Type,
                                                     const clang::Type *DType,
                                                     EmitRanges ER);

  /// \brief Builds a stable node ID for `Type`.
  /// \param Type The type that is being identified. If its location is valid
  /// and `ER` is `EmitRanges::Yes`, notifies the attached `GraphObserver` about
  /// the location of constituent elements.
  /// \param QT The deduced form of `Type`. (May be `Type.getType()`).
  /// \param ER whether to notify the `GraphObserver` about source text ranges
  /// for types.
  /// \return The Node ID for `Type`.
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForType(const clang::TypeLoc &Type,
                                                     const clang::QualType &QT,
                                                     EmitRanges ER);

  /// \brief Builds a stable node ID for `Type`.
  /// \param Type The type that is being identified. If its location is valid
  /// and `ER` is `EmitRanges::Yes`, notifies the attached `GraphObserver` about
  /// the location of constituent elements.
  /// \param ER whether to notify the `GraphObserver` about source text ranges
  /// for types.
  /// \return The Node ID for `Type`.
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForType(const clang::TypeLoc &Type,
                                                     EmitRanges ER);

  /// \brief Builds a stable node ID for `QT`.
  /// \param QT The type that is being identified.
  /// \return The Node ID for `QT`.
  ///
  /// This function will invent a `TypeLoc` with an invalid location.
  /// If you can provide a `TypeLoc` for the type, it is better to use
  /// `BuildNodeIdForType(const clang::TypeLoc, EmitRanges)`.
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForType(const clang::QualType &QT);

  /// \brief Builds a stable node ID for the given `TemplateName`.
  MaybeFew<GraphObserver::NodeId>
  BuildNodeIdForTemplateName(const clang::TemplateName &Name,
                             clang::SourceLocation L);

  /// \brief Builds a stable node ID for the given `TemplateArgument`.
  MaybeFew<GraphObserver::NodeId>
  BuildNodeIdForTemplateArgument(const clang::TemplateArgumentLoc &Arg,
                                 EmitRanges ER);

  /// \brief Builds a stable node ID for the given `TemplateArgument`.
  MaybeFew<GraphObserver::NodeId>
  BuildNodeIdForTemplateArgument(const clang::TemplateArgument &Arg,
                                 clang::SourceLocation L);

  /// \brief Builds a stable node ID for `Stmt`.
  ///
  /// This mechanism for generating IDs should only be used in contexts where
  /// identifying statements through source locations/wraith contexts is not
  /// possible (e.g., in implicit code).
  ///
  /// \param Decl The statement that is being identified
  /// \return The node for `Stmt` if the statement was implicit; otherwise,
  /// None.
  MaybeFew<GraphObserver::NodeId>
  BuildNodeIdForImplicitStmt(const clang::Stmt *Stmt);

  /// \brief Builds a stable node ID for `Decl`.
  ///
  /// \param Decl The declaration that is being identified.
  /// \return The node for `Decl`.
  GraphObserver::NodeId BuildNodeIdForDecl(const clang::Decl *Decl);

  /// \brief Builds a stable node ID for `TND`.
  ///
  /// \param Decl The declaration that is being identified.
  MaybeFew<GraphObserver::NodeId>
  BuildNodeIdForTypedefNameDecl(const clang::TypedefNameDecl *TND);

  /// \brief Builds a stable node ID for `Decl`.
  ///
  /// There is not a one-to-one correspondence between `Decl`s and nodes.
  /// Certain `Decl`s, like `TemplateTemplateParmDecl`, are split into a
  /// node representing the parameter and a node representing the kind of
  /// the abstraction. The primary node is returned by the normal
  /// `BuildNodeIdForDecl` function.
  ///
  /// \param Decl The declaration that is being identified.
  /// \param Index The index of the sub-id to generate.
  ///
  /// \return A stable node ID for `Decl`'s `Index`th subnode.
  GraphObserver::NodeId BuildNodeIdForDecl(const clang::Decl *Decl,
                                           unsigned Index);

  /// \brief Categorizes the name of `Decl` according to the equivalence classes
  /// defined by `GraphObserver::NameId::NameEqClass`.
  GraphObserver::NameId::NameEqClass
  BuildNameEqClassForDecl(const clang::Decl *Decl);

  /// \brief Builds a stable name ID for the name of `Decl`.
  ///
  /// \param Decl The declaration that is being named.
  /// \return The name for `Decl`.
  GraphObserver::NameId BuildNameIdForDecl(const clang::Decl *Decl);

  /// \brief Builds a NodeId for the given dependent name.
  ///
  /// \param NNS The qualifier on the name.
  /// \param Id The name itself.
  /// \param IdLoc The name's location.
  /// \param Root If provided, the primary NodeId is morally prepended to `NNS`
  /// such that the dependent name is lookup(lookup*(Root, NNS), Id).
  /// \param ER If `EmitRanges::Yes`, records ranges for syntactic elements.
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForDependentName(
      const clang::NestedNameSpecifierLoc &NNS,
      const clang::DeclarationName &Id, const clang::SourceLocation IdLoc,
      const MaybeFew<GraphObserver::NodeId> &Root, EmitRanges ER);

  /// \brief Is `VarDecl` a definition?
  ///
  /// A parameter declaration is considered a definition if it appears as part
  /// of a function definition; otherwise it is always a declaration. This
  /// differs from the C++ Standard, which treats these parameters as
  /// definitions (basic.scope.proto).
  static bool IsDefinition(const clang::VarDecl *VD);

  /// \brief Is `FunctionDecl` a definition?
  static bool IsDefinition(const clang::FunctionDecl *FD);

  /// \brief Gets a range for the name of a declaration, whether that name is a
  /// single token or otherwise.
  ///
  /// The returned range is a best-effort attempt to cover the "name" of
  /// the entity as written in the source code.
  clang::SourceRange
  RangeForNameOfDeclaration(const clang::NamedDecl *Decl) const;

  /// \brief Gets a suitable range for an AST entity from the `StartLocation`.
  ///
  /// Note: if the AST entity is a declaration, use `RangeForNameOfDeclaration`,
  /// as that can use more semantic information to determine a better range in
  /// some cases.
  ///
  /// The returned range is a best-effort attempt to cover the "name" of
  /// the entity as written in the source code.
  ///
  /// This range does double duty, being used for a "source link" as well as
  /// to represent the semantic range of what's being used. Therefore,
  /// if the entity is not in a macro expansion, or it comes from a top-level
  /// macro argument and itself is not expanded from a macro, Kythe has found
  /// it best to span just the entity's name -- in most cases, that's a
  /// single token.  Otherwise, when the entity comes from a macro definition
  /// (and not from a top-level macro argument), the resulting range is a
  /// 0-width span (a point), so that no source link will be created.
  ///
  /// Note: the definition of "suitable" is in the context of the AST visitor.
  /// Being suitable may mean something different to the preprocessor.
  clang::SourceRange RangeForASTEntityFromSourceLocation(
      clang::SourceLocation StartLocation) const;

  /// \brief Gets a range for a token from the location specified by
  /// `StartLocation`.
  ///
  /// Assumes the `StartLocation` is a file location (i.e., isFileID returning
  /// true) because we want the range of a token in the context of the
  /// original file.
  clang::SourceRange RangeForSingleTokenFromSourceLocation(
      clang::SourceLocation StartLocation) const;

  /// \brief Gets a suitable range to represent the name of some `operator???`,
  /// whether it's an overloaded operator or a conversion operator.
  ///
  /// For conversion operators, the range is the given `OperatorTokenRange`.
  ///
  /// For overloaded operators, the range covers the whole operator name
  /// (e.g., "operator++" or "operator[]").  Note: There's currently a bug
  /// in that operators new/delete/new[]/delete[] get single-token ranges.
  ///
  /// The argument `OperatorTokenRange` should span the text of the
  /// `operator` keyword.
  clang::SourceRange
  RangeForOperatorName(const clang::SourceRange &OperatorTokenRange) const;

  /// Consume a token of the `ExpectedKind` from the `StartLocation`,
  /// returning the location after that token on success and an invalid
  /// location otherwise.
  clang::SourceLocation ConsumeToken(clang::SourceLocation StartLocation,
                                     clang::tok::TokenKind ExpectedKind) const;

  /// \brief Using the source manager and language options of the `Observer`,
  /// find the location at the end of the token starting at `StartLocation`.
  clang::SourceLocation
  GetLocForEndOfToken(clang::SourceLocation StartLocation) const;

  bool TraverseClassTemplateDecl(clang::ClassTemplateDecl *TD);
  bool TraverseClassTemplateSpecializationDecl(
      clang::ClassTemplateSpecializationDecl *TD);
  bool TraverseClassTemplatePartialSpecializationDecl(
      clang::ClassTemplatePartialSpecializationDecl *TD);

  bool TraverseVarTemplateDecl(clang::VarTemplateDecl *VD);
  bool TraverseVarTemplateSpecializationDecl(
      clang::VarTemplateSpecializationDecl *VD);
  bool ForceTraverseVarTemplateSpecializationDecl(
      clang::VarTemplateSpecializationDecl *VD);
  bool TraverseVarTemplatePartialSpecializationDecl(
      clang::VarTemplatePartialSpecializationDecl *TD);

  bool TraverseFunctionTemplateDecl(clang::FunctionTemplateDecl *FTD);

  bool TraverseTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl *TATD);

  bool shouldVisitTemplateInstantiations() const {
    return TemplateMode == BehaviorOnTemplates::VisitInstantiations;
  }
  bool shouldVisitImplicitCode() const { return true; }
  // Disables data recursion. We intercept Traverse* methods in the RAV, which
  // are not triggered during data recursion.
  bool shouldUseDataRecursionFor(clang::Stmt *S) const { return false; }

  /// \brief Records the range of a definition. If the range cannot be placed
  /// somewhere inside a source file, no record is made.
  void MaybeRecordDefinitionRange(const MaybeFew<GraphObserver::Range> &R,
                                  const GraphObserver::NodeId &Id);

  /// \brief Returns the attached GraphObserver.
  GraphObserver &getGraphObserver() { return Observer; }

  /// Returns `SR` as a `Range` in this `RecursiveASTVisitor`'s current
  /// RangeContext.
  MaybeFew<GraphObserver::Range>
  ExplicitRangeInCurrentContext(const clang::SourceRange &SR);

  /// If `Implicit` is true, returns `Id` as an implicit Range; otherwise,
  /// returns `SR` as a `Range` in this `RecursiveASTVisitor`'s current
  /// RangeContext.
  MaybeFew<GraphObserver::Range>
  RangeInCurrentContext(bool Implicit, const GraphObserver::NodeId &Id,
                        const clang::SourceRange &SR);

  /// If `Id` is some NodeId, returns it as an implicit Range; otherwise,
  /// returns `SR` as a `Range` in this `RecursiveASTVisitor`'s current
  /// RangeContext.
  MaybeFew<GraphObserver::Range>
  RangeInCurrentContext(const MaybeFew<GraphObserver::NodeId> &Id,
                        const clang::SourceRange &SR) {
    if (auto &PrimaryId = Id) {
      return GraphObserver::Range(PrimaryId.primary());
    }
    return ExplicitRangeInCurrentContext(SR);
  }

private:
  /// Whether we should stop on missing cases or continue on.
  BehaviorOnUnimplemented IgnoreUnimplemented;

  /// Should we visit template instantiations?
  BehaviorOnTemplates TemplateMode;

  /// Should we emit all data?
  enum Verbosity Verbosity;

  NullGraphObserver NullObserver;
  GraphObserver &Observer;
  clang::ASTContext &Context;

  /// \brief The result of calling into the lexer.
  enum class LexerResult {
    Failure, ///< The operation failed.
    Success  ///< The operation completed.
  };

  /// \brief Using the `Observer`'s preprocessor, relexes the token at the
  /// specified location. Ignores whitespace.
  /// \param StartLocation Where to begin lexing.
  /// \param Token The token to overwrite.
  /// \return `Failure` if there was a failure, `Success` on success.
  LexerResult getRawToken(clang::SourceLocation StartLocation,
                          clang::Token &Token) const {
    return Observer.getPreprocessor()->getRawToken(StartLocation, Token,
                                                   true /* ignoreWhiteSpace */)
               ? LexerResult::Failure
               : LexerResult::Success;
  }

  /// \brief Builds a semantic hash of the given `Decl`, which should look
  /// like a `TemplateDecl` (eg, a `TemplateDecl` itself or a partial
  /// specialization).
  template <typename TemplateDeclish>
  uint64_t SemanticHashTemplateDeclish(const TemplateDeclish *Decl);

  /// \brief Builds a semantic hash of the given `TemplateArgumentList`.
  uint64_t SemanticHash(const clang::TemplateArgumentList *TAL);

  /// \brief Builds a semantic hash of the given `TemplateArgument`.
  uint64_t SemanticHash(const clang::TemplateArgument &TA);

  /// \brief Builds a semantic hash of the given `TemplateName`.
  uint64_t SemanticHash(const clang::TemplateName &TN);

  /// \brief Builds a semantic hash of the given `Type`, such that
  /// if T and T' are similar types, SH(T) == SH(T'). Note that the type is
  /// always canonicalized before its hash is taken.
  uint64_t SemanticHash(const clang::QualType &T);

  /// \brief Builds a semantic hash of the given `RecordDecl`, such that
  /// if R and R' are similar records, SH(R) == SH(R'). This notion of
  /// similarity is meant to join together definitions copied and pasted
  /// across different translation units. As it is at best an approximation,
  /// it should be paired with the spelled-out name of the object being declared
  /// to form an identifying token.
  uint64_t SemanticHash(const clang::RecordDecl *RD);

  /// \brief Builds a semantic hash of the given `EnumDecl`, such that
  /// if E and E' are similar records, SH(E) == SH(E'). This notion of
  /// similarity is meant to join together definitions copied and pasted
  /// across different translation units. As it is at best an approximation,
  /// it should be paired with the spelled-out name of the object being declared
  /// to form an identifying token.
  uint64_t SemanticHash(const clang::EnumDecl *ED);

  /// \brief Gets a format string for `ND`.
  std::string GetFormat(const clang::NamedDecl *ND);

  /// \brief Attempts to add a format string representation of `ND` to
  /// `Ostream`.
  /// \return true on success; false on failure.
  bool AddFormatToStream(llvm::raw_string_ostream &Ostream,
                         const clang::NamedDecl *ND);

  /// \brief Attempts to find the ID of the first parent of `Decl` for
  /// generating a format string.
  MaybeFew<GraphObserver::NodeId> GetParentForFormat(const clang::Decl *D);

  /// \brief Attempts to add some representation of `ND` to `Ostream`.
  /// \return true on success; false on failure.
  bool AddNameToStream(llvm::raw_string_ostream &Ostream,
                       const clang::NamedDecl *ND);

  MaybeFew<GraphObserver::NodeId>
  ApplyBuiltinTypeConstructor(const char *BuiltinName,
                              const MaybeFew<GraphObserver::NodeId> &Param);

  /// \brief Ascribes a type to `AscribeTo`.
  /// \param Type The `TypeLoc` referring to the type
  /// \param DType A possibly deduced type (or simply Type->getType()).
  /// \param AscribeTo The node to which the type should be ascribed.
  ///
  /// `auto` does not update TypeSourceInfo records after deduction, so
  /// a deduced `auto` in the source text will appear to be undeduced.
  /// In this case, it's useful to query the object being ascribed for its
  /// unlocated QualType, as this does get updated.
  void AscribeSpelledType(const clang::TypeLoc &Type,
                          const clang::QualType &DType,
                          const GraphObserver::NodeId &AscribeTo);

  /// \brief Returns the parents of the given node, along with the index
  /// at which the node appears underneath each parent.
  ///
  /// Note that this will lazily compute the parents of all nodes
  /// and store them for later retrieval. Thus, the first call is O(n)
  /// in the number of AST nodes.
  ///
  /// Caveats and FIXMEs:
  /// Calculating the parent map over all AST nodes will need to load the
  /// full AST. This can be undesirable in the case where the full AST is
  /// expensive to create (for example, when using precompiled header
  /// preambles). Thus, there are good opportunities for optimization here.
  /// One idea is to walk the given node downwards, looking for references
  /// to declaration contexts - once a declaration context is found, compute
  /// the parent map for the declaration context; if that can satisfy the
  /// request, loading the whole AST can be avoided. Note that this is made
  /// more complex by statements in templates having multiple parents - those
  /// problems can be solved by building closure over the templated parts of
  /// the AST, which also avoids touching large parts of the AST.
  /// Additionally, we will want to add an interface to already give a hint
  /// where to search for the parents, for example when looking at a statement
  /// inside a certain function.
  ///
  /// 'NodeT' can be one of Decl, Stmt, Type, TypeLoc,
  /// NestedNameSpecifier or NestedNameSpecifierLoc.
  template <typename NodeT>
  IndexedParentVector getIndexedParents(const NodeT &Node) {
    return getIndexedParents(
        clang::ast_type_traits::DynTypedNode::create(Node));
  }

  IndexedParentVector
  getIndexedParents(const clang::ast_type_traits::DynTypedNode &Node);
  /// A map from memoizable DynTypedNodes to their parent nodes
  /// and their child indices with respect to those parents.
  /// Filled on the first call to `getIndexedParents`.
  std::unique_ptr<IndexedParentMap> AllParents;

  /// \brief Deallocates `AllParents`.
  void deleteAllParents();

  /// Records information about the template `Template` wrapping the node
  /// `BodyId`, including the edge linking the template and its body. Returns
  /// the `NodeId` for the dominating template.
  template <typename TemplateDeclish>
  GraphObserver::NodeId RecordTemplate(const TemplateDeclish *Template,
                                       const GraphObserver::NodeId &BodyId);

  /// \brief Fills `ArgNodeIds` with pointers to `NodeId`s for a template
  /// argument list.
  /// \param `ArgsAsWritten` Use this argument list if it isn't null.
  /// \param `Args` Use this argument list if `ArgsAsWritten` is null.
  /// \param `ArgIds` Vector into which pointers in `ArgNodeIds` will point.
  /// \param `ArgNodeIds` Vector of pointers to the result type list.
  /// \return true on success; if false, `ArgNodeIds` is invalid.
  bool BuildTemplateArgumentList(
      const clang::ASTTemplateArgumentListInfo *ArgsAsWritten,
      const clang::TemplateArgumentList *Args,
      std::vector<GraphObserver::NodeId> &ArgIds,
      std::vector<const GraphObserver::NodeId *> &ArgNodeIds);

  /// Dumps information about `TypeContext` to standard error when looking for
  /// an entry at (`Depth`, `Index`).
  void DumpTypeContext(unsigned Depth, unsigned Index);

  /// \brief Attempts to add a childof edge between DeclNode and its parent.
  /// \param Decl The (outer, in the case of a template) decl.
  /// \param DeclNode The (outer) `NodeId` for `Decl`.
  void AddChildOfEdgeToDeclContext(const clang::Decl *Decl,
                                   const GraphObserver::NodeId DeclNode);

  /// Points at the inner node of the DeclContext, if it's a template.
  /// Otherwise points at the DeclContext as a Decl.
  MaybeFew<GraphObserver::NodeId>
  BuildNodeIdForDeclContext(const clang::DeclContext *DC);

  /// Avoid regenerating type node IDs and keep track of where we are while
  /// generating node IDs for recursive types. The key is opaque and
  /// makes sense only within the implementation of this class.
  std::unordered_map<int64_t, MaybeFew<GraphObserver::NodeId>> TypeNodes;

  /// The current type variable context for the visitor (indexed by depth).
  std::vector<clang::TemplateParameterList *> TypeContext;

  /// At least 1 NodeId.
  using SomeNodes = llvm::SmallVector<GraphObserver::NodeId, 1>;

  /// A stack of ID groups to use when assigning blame for references (such as
  /// function calls).
  std::vector<SomeNodes> BlameStack;

  /// Blames a call to `Callee` at `Range` on everything at the top of
  /// `BlameStack` (or does nothing if there's nobody to blame).
  void RecordCallEdges(const GraphObserver::Range &Range,
                       const GraphObserver::NodeId &Callee);

  /// \brief The current context for constructing `GraphObserver::Range`s.
  ///
  /// This is used whenever we enter a context where a section of source
  /// code might have different meaning depending on assignments to
  /// type variables (or, potentially eventually, preprocessor variables).
  /// The `RangeContext` does not need to be extended when the meaning of
  /// a section of code (given the current context) is unambiguous; for example,
  /// the programmer must write down explicit specializations of template
  /// classes (so they have distinct ranges), but the programmer does *not*
  /// write down implicit specializations (so the context must be extended to
  /// give them distinct ranges).
  std::vector<GraphObserver::NodeId> RangeContext;

  /// \brief Maps known Decls to their NodeIds.
  llvm::DenseMap<const clang::Decl *, GraphObserver::NodeId> DeclToNodeId;

  /// \brief Maps EnumDecls to semantic hashes.
  llvm::DenseMap<const clang::EnumDecl *, uint64_t> EnumToHash;

  /// \brief Enabled library-specific callbacks.
  const LibrarySupports &Supports;

  /// \brief The `Sema` instance to use.
  clang::Sema &Sema;
};

/// \brief An `ASTConsumer` that passes events to a `GraphObserver`.
class IndexerASTConsumer : public clang::SemaConsumer {
public:
  explicit IndexerASTConsumer(GraphObserver *GO, BehaviorOnUnimplemented B,
                              BehaviorOnTemplates T, Verbosity V,
                              const LibrarySupports &S)
      : Observer(GO), IgnoreUnimplemented(B), TemplateMode(T), Verbosity(V),
        Supports(S) {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    CHECK(Sema != nullptr);
    IndexerASTVisitor Visitor(Context, IgnoreUnimplemented, TemplateMode,
                              Verbosity, Supports, *Sema, Observer);
    {
      ProfileBlock block(Observer->getProfilingCallback(), "traverse_tu");
      Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }
  }

  void InitializeSema(clang::Sema &S) override { Sema = &S; }

  void ForgetSema() override { Sema = nullptr; }

private:
  GraphObserver *const Observer;
  /// Whether we should stop on missing cases or continue on.
  BehaviorOnUnimplemented IgnoreUnimplemented;
  /// Whether we should visit template instantiations.
  BehaviorOnTemplates TemplateMode;
  /// Whether we should emit all data.
  enum Verbosity Verbosity;
  /// Which library supports are enabled.
  const LibrarySupports &Supports;
  /// The active Sema instance.
  clang::Sema *Sema;
};

} // namespace kythe

#endif // KYTHE_CXX_INDEXER_CXX_INDEXER_AST_HOOKS_H_
