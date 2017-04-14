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
#include <unordered_set>
#include <utility>

#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTTypeTraits.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Index/USRGeneration.h"
#include "clang/Sema/SemaConsumer.h"
#include "clang/Sema/Template.h"
#include "glog/logging.h"

#include "GraphObserver.h"
#include "IndexerLibrarySupport.h"
#include "indexer_worklist.h"
#include "marked_source.h"

namespace kythe {

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

using IndexedParentMap =
    llvm::DenseMap<const void *,
                   llvm::PointerIntPair<IndexedParent *, 1, bool>>;

/// \return `true` if truncating tree traversal at `D` is safe, provided that
/// `D` has been traversed previously.
bool IsClaimableForTraverse(const clang::Decl *D);

/// \return `true` if truncating tree traversal at `S` is safe, provided that
/// `S` has been traversed previously.
inline bool IsClaimableForTraverse(const clang::Stmt *S) { return false; }

/// FIXME: Currently only builds up the map using \c Stmt and \c Decl nodes.
/// TODO(zarko): Is this necessary to change for naming?
class IndexedParentASTVisitor
    : public clang::RecursiveASTVisitor<IndexedParentASTVisitor> {
 public:
  /// \brief Builds and returns the translation unit's indexed parent map.
  ///
  /// The caller takes ownership of the returned `IndexedParentMap`.
  static std::unique_ptr<IndexedParentMap> buildMap(
      clang::TranslationUnitDecl &TU) {
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
    if (!Node) return true;
    if (!ParentStack.empty()) {
      auto &NodeOrVector = (*Parents)[Node];
      if (NodeOrVector.getPointer() == nullptr) {
        // It's not useful to store more than one parent.
        NodeOrVector.setPointer(new IndexedParent(ParentStack.back()));
      }
    }
    ParentStack.push_back(
        {clang::ast_type_traits::DynTypedNode::create(*Node), 0});
    bool SavedClaimableAtThisDepth = ClaimableAtThisDepth;
    ClaimableAtThisDepth = false;  // for depth + 1
    bool Result = traverse(Node);
    if (ClaimableAtThisDepth || IsClaimableForTraverse(Node)) {
      ClaimableAtThisDepth = true;  // for depth
      (*Parents)[Node].setInt(1);
    } else {
      ClaimableAtThisDepth = SavedClaimableAtThisDepth;  // restore depth
    }
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
  bool ClaimableAtThisDepth = false;

  friend class RecursiveASTVisitor<IndexedParentASTVisitor>;
};

/// \brief Specifies whether uncommonly-used data should be dropped.
enum Verbosity : bool {
  Classic = true,  ///< Emit all data.
  Lite = false     ///< Emit only common data.
};

/// \brief Specifies what the indexer should do if it encounters a case it
/// doesn't understand.
enum BehaviorOnUnimplemented : bool {
  Abort = false,   ///< Stop indexing and exit with an error.
  Continue = true  ///< Continue indexing, possibly emitting less data.
};

/// \brief Specifies what the indexer should do with template instantiations.
enum BehaviorOnTemplates : bool {
  SkipInstantiations = false,  ///< Don't visit template instantiations.
  VisitInstantiations = true   ///< Visit template instantiations.
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

/// \brief Used internally to check whether parts of the AST can be ignored.
class PruneCheck;

/// \brief An AST visitor that extracts information for a translation unit and
/// writes it to a `GraphObserver`.
class IndexerASTVisitor : public clang::RecursiveASTVisitor<IndexerASTVisitor> {
 public:
  IndexerASTVisitor(clang::ASTContext &C, BehaviorOnUnimplemented B,
                    BehaviorOnTemplates T, Verbosity V,
                    const LibrarySupports &S,
                    clang::Sema &Sema, std::function<bool()> ShouldStopIndexing,
                    GraphObserver *GO = nullptr)
      : IgnoreUnimplemented(B),
        TemplateMode(T),
        Verbosity(V),
        Observer(GO ? *GO : NullObserver),
        Context(C),
        Supports(S),
        Sema(Sema),
        MarkedSources(&Sema, &Observer),
        ShouldStopIndexing(std::move(ShouldStopIndexing)) {}

  ~IndexerASTVisitor() { deleteAllParents(); }

  bool VisitDecl(const clang::Decl *Decl);
  bool VisitFieldDecl(const clang::FieldDecl *Decl);
  bool VisitVarDecl(const clang::VarDecl *Decl);
  bool VisitNamespaceDecl(const clang::NamespaceDecl *Decl);
  bool VisitDeclRefExpr(const clang::DeclRefExpr *DRE);
  bool VisitUnaryExprOrTypeTraitExpr(const clang::UnaryExprOrTypeTraitExpr *E);
  bool VisitCXXConstructExpr(const clang::CXXConstructExpr *E);
  bool VisitCXXDeleteExpr(const clang::CXXDeleteExpr *E);
  bool VisitCXXNewExpr(const clang::CXXNewExpr *E);
  bool VisitCXXPseudoDestructorExpr(const clang::CXXPseudoDestructorExpr *E);
  bool VisitCXXUnresolvedConstructExpr(
      const clang::CXXUnresolvedConstructExpr *E);
  bool VisitCallExpr(const clang::CallExpr *Expr);
  bool VisitMemberExpr(const clang::MemberExpr *Expr);
  bool VisitCXXDependentScopeMemberExpr(
      const clang::CXXDependentScopeMemberExpr *Expr);

  // Visit the subtypes of TypedefNameDecl individually because we want to do
  // something different with ObjCTypeParamDecl.
  bool VisitTypedefDecl(const clang::TypedefDecl *Decl);
  bool VisitTypeAliasDecl(const clang::TypeAliasDecl *Decl);
  bool VisitObjCTypeParamDecl(const clang::ObjCTypeParamDecl *Decl);

  bool VisitRecordDecl(const clang::RecordDecl *Decl);
  bool VisitEnumDecl(const clang::EnumDecl *Decl);
  bool VisitEnumConstantDecl(const clang::EnumConstantDecl *Decl);
  bool VisitFunctionDecl(clang::FunctionDecl *Decl);
  bool TraverseDecl(clang::Decl *Decl);

  // Objective C specific nodes
  bool VisitObjCPropertyImplDecl(const clang::ObjCPropertyImplDecl *Decl);
  bool VisitObjCCompatibleAliasDecl(const clang::ObjCCompatibleAliasDecl *Decl);
  bool VisitObjCCategoryDecl(const clang::ObjCCategoryDecl *Decl);
  bool VisitObjCImplementationDecl(
      const clang::ObjCImplementationDecl *ImplDecl);
  bool VisitObjCCategoryImplDecl(const clang::ObjCCategoryImplDecl *ImplDecl);
  bool VisitObjCInterfaceDecl(const clang::ObjCInterfaceDecl *Decl);
  bool VisitObjCProtocolDecl(const clang::ObjCProtocolDecl *Decl);
  bool VisitObjCMethodDecl(const clang::ObjCMethodDecl *Decl);
  bool VisitObjCPropertyDecl(const clang::ObjCPropertyDecl *Decl);
  bool VisitObjCIvarRefExpr(const clang::ObjCIvarRefExpr *IRE);
  bool VisitObjCMessageExpr(const clang::ObjCMessageExpr *Expr);
  bool VisitObjCPropertyRefExpr(const clang::ObjCPropertyRefExpr *Expr);

  // TODO(salguarnieri) We could link something here (the square brackets?) to
  // the setter and getter methods
  //   bool VisitObjCSubscriptRefExpr(const clang::ObjCSubscriptRefExpr *Expr);

  // TODO(salguarnieri) delete this comment block when we have more objective-c
  // support implemented.
  //
  //  Visitors that are left to their default behavior because we do not need
  //  to take any action while visiting them.
  //   bool VisitObjCDictionaryLiteral(const clang::ObjCDictionaryLiteral *D);
  //   bool VisitObjCArrayLiteral(const clang::ObjCArrayLiteral *D);
  //   bool VisitObjCBoolLiteralExpr(const clang::ObjCBoolLiteralExpr *D);
  //   bool VisitObjCStringLiteral(const clang::ObjCStringLiteral *D);
  //   bool VisitObjCEncodeExpr(const clang::ObjCEncodeExpr *Expr);
  //   bool VisitObjCBoxedExpr(const clang::ObjCBoxedExpr *Expr);
  //   bool VisitObjCSelectorExpr(const clang::ObjCSelectorExpr *Expr);
  //   bool VisitObjCIndirectCopyRestoreExpr(
  //    const clang::ObjCIndirectCopyRestoreExpr *Expr);
  //   bool VisitObjCIsaExpr(const clang::ObjCIsaExpr *Expr);
  //
  //  We visit the subclasses of ObjCContainerDecl so there is nothing to do.
  //   bool VisitObjCContainerDecl(const clang::ObjCContainerDecl *D);
  //
  //  We visit the subclasses of ObjCImpleDecl so there is nothing to do.
  //   bool VisitObjCImplDecl(const clang::ObjCImplDecl *D);
  //
  //  There is nothing specific we need to do for blocks. The recursive visitor
  //  will visit the components of them correctly.
  //   bool VisitBlockDecl(const clang::BlockDecl *Decl);
  //   bool VisitBlockExpr(const clang::BlockExpr *Expr);

  /// \brief For functions that support it, controls the emission of range
  /// information.
  enum class EmitRanges {
    No,  ///< Don't emit range information.
    Yes  ///< Emit range information when it's available.
  };

  // Objective C methods don't have TypeSourceInfo so we must construct a type
  // for the methods to be used in the graph.
  MaybeFew<GraphObserver::NodeId> CreateObjCMethodTypeNode(
      const clang::ObjCMethodDecl *MD, EmitRanges ER);

  /// \brief Builds a stable node ID for a compile-time expression.
  /// \param Expr The expression to represent.
  /// \param ER Whether to notify the `GraphObserver` about source text
  /// ranges for expressions.
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForExpr(const clang::Expr *Expr,
                                                     EmitRanges ER);

  /// \brief Builds a stable node ID for `Type`.
  /// \param TypeLoc The type that is being identified. If its location is valid
  /// and `ER` is `EmitRanges::Yes`, notifies the attached `GraphObserver` about
  /// the location of constituent elements.
  /// \param DType The deduced form of `Type`. (May be `Type.getTypePtr()`).
  /// \param ER whether to notify the `GraphObserver` about source text ranges
  /// for types.
  /// \return The Node ID for `Type`.
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForType(
      const clang::TypeLoc &TypeLoc, const clang::Type *DType, EmitRanges ER);

  /// \brief Builds a stable node ID for `Type`.
  /// \param TypeLoc The type that is being identified. If its location is valid
  /// and `ER` is `EmitRanges::Yes`, notifies the attached `GraphObserver` about
  /// the location of constituent elements.
  /// \param DType The deduced form of `Type`. (May be `Type.getTypePtr()`).
  /// \param ER whether to notify the `GraphObserver` about source text ranges
  /// for types.
  /// \param SR source range to use for the type location. This will be used
  /// instead of the range in TypeLoc.
  /// \return The Node ID for `Type`.
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForType(
      const clang::TypeLoc &TypeLoc, const clang::Type *DType, EmitRanges ER,
      clang::SourceRange SR);

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
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForTemplateName(
      const clang::TemplateName &Name, clang::SourceLocation L);

  /// \brief Builds a stable node ID for the given `TemplateArgument`.
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForTemplateArgument(
      const clang::TemplateArgumentLoc &Arg, EmitRanges ER);

  /// \brief Builds a stable node ID for the given `TemplateArgument`.
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForTemplateArgument(
      const clang::TemplateArgument &Arg, clang::SourceLocation L);

  /// \brief Builds a stable node ID for `Stmt`.
  ///
  /// This mechanism for generating IDs should only be used in contexts where
  /// identifying statements through source locations/wraith contexts is not
  /// possible (e.g., in implicit code).
  ///
  /// \param Decl The statement that is being identified
  /// \return The node for `Stmt` if the statement was implicit; otherwise,
  /// None.
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForImplicitStmt(
      const clang::Stmt *Stmt);

  /// \brief Builds a stable node ID for `Decl`'s tapp if it's an implicit
  /// template instantiation.
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForImplicitTemplateInstantiation(
      const clang::Decl *Decl);

  /// \brief Builds a stable node ID for an external reference to `Decl`.
  ///
  /// This is equivalent to BuildNodeIdForDecl for Decls that are not
  /// implicit template instantiations; otherwise, it returns the `NodeId`
  /// for the tapp node for the instantiation.
  ///
  /// \param Decl The declaration that is being identified.
  /// \return The node for `Decl`.
  GraphObserver::NodeId BuildNodeIdForRefToDecl(const clang::Decl *Decl);

  /// \brief Builds a stable node ID for `Decl`.
  ///
  /// \param Decl The declaration that is being identified.
  /// \return The node for `Decl`.
  GraphObserver::NodeId BuildNodeIdForDecl(const clang::Decl *Decl);

  /// \brief Builds a stable node ID for `TND`.
  ///
  /// \param Decl The declaration that is being identified.
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForTypedefNameDecl(
      const clang::TypedefNameDecl *TND);

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
  GraphObserver::NameId::NameEqClass BuildNameEqClassForDecl(
      const clang::Decl *Decl);

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
      const clang::NestedNameSpecifierLoc &NNS, const clang::DeclarationName &Id,
      const clang::SourceLocation IdLoc,
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
  clang::SourceRange RangeForNameOfDeclaration(
      const clang::NamedDecl *Decl) const;

  /// Consume a token of the `ExpectedKind` from the `StartLocation`,
  /// returning the range for that token on success and an invalid
  /// range otherwise.
  ///
  /// The begin location for the returned range may be different than
  /// StartLocation. For example, this can happen if StartLocation points to
  /// whitespace before the start of the token.
  clang::SourceRange ConsumeToken(clang::SourceLocation StartLocation,
                                  clang::tok::TokenKind ExpectedKind) const;

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

  bool TraverseFunctionDecl(clang::FunctionDecl *FD);
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

  /// \brief Returns the ASTContext.
  const clang::ASTContext &getASTContext() { return Context; }

  /// Returns `SR` as a `Range` in this `RecursiveASTVisitor`'s current
  /// RangeContext.
  MaybeFew<GraphObserver::Range> ExplicitRangeInCurrentContext(
      const clang::SourceRange &SR);

  /// If `Implicit` is true, returns `Id` as an implicit Range; otherwise,
  /// returns `SR` as a `Range` in this `RecursiveASTVisitor`'s current
  /// RangeContext.
  MaybeFew<GraphObserver::Range> RangeInCurrentContext(
      bool Implicit, const GraphObserver::NodeId &Id,
      const clang::SourceRange &SR);

  /// If `Id` is some NodeId, returns it as an implicit Range; otherwise,
  /// returns `SR` as a `Range` in this `RecursiveASTVisitor`'s current
  /// RangeContext.
  MaybeFew<GraphObserver::Range> RangeInCurrentContext(
      const MaybeFew<GraphObserver::NodeId> &Id, const clang::SourceRange &SR);

  void RunJob(std::unique_ptr<IndexJob> JobToRun) {
    Job = std::move(JobToRun);
    if (Job->IsDeclJob) {
      TraverseDecl(Job->Decl);
    } else {
      // There is no declaration attached to a top-level file comment.
      HandleFileLevelComments(Job->FileId, Job->FileNode);
    }
  }

  const IndexJob &getCurrentJob() {
    CHECK(Job != nullptr);
    return *Job;
  }

  void Work(clang::Decl *InitialDecl,
            std::unique_ptr<IndexerWorklist> NewWorklist) {
    Worklist = std::move(NewWorklist);
    Worklist->EnqueueJob(llvm::make_unique<IndexJob>(InitialDecl));
    while (!ShouldStopIndexing() && Worklist->DoWork())
      ;
    Observer.iterateOverClaimedFiles(
        [this, InitialDecl](clang::FileID Id,
                            const GraphObserver::NodeId &FileNode) {
          RunJob(llvm::make_unique<IndexJob>(InitialDecl, Id, FileNode));
          return !ShouldStopIndexing();
        });
    Worklist.reset();
  }

  /// \brief Provides execute-only access to ShouldStopIndexing. Should be used
  /// from the same thread that's walking the AST.
  bool shouldStopIndexing() const { return ShouldStopIndexing(); }

  /// Blames a call to `Callee` at `Range` on everything at the top of
  /// `BlameStack` (or does nothing if there's nobody to blame).
  void RecordCallEdges(const GraphObserver::Range &Range,
                       const GraphObserver::NodeId &Callee);

 private:
  friend class PruneCheck;

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
    Failure,  ///< The operation failed.
    Success   ///< The operation completed.
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

  /// \brief Handle the file-level comments for `Id` with node ID `FileId`.
  void HandleFileLevelComments(clang::FileID Id,
                               const GraphObserver::NodeId &FileId);

  /// \brief Emit data for `Comment` that documents `DocumentedNode`, using
  /// `DC` for lookups.
  void VisitComment(const clang::RawComment *Comment,
                    const clang::DeclContext *DC,
                    const GraphObserver::NodeId &DocumentedNode);

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

  uint64_t SemanticHash(const clang::Selector &S);

  /// \brief Attempts to find the ID of the first parent of `Decl` for
  /// attaching a `childof` relationship.
  MaybeFew<GraphObserver::NodeId> GetDeclChildOf(const clang::Decl *D);

  /// \brief Attempts to add some representation of `ND` to `Ostream`.
  /// \return true on success; false on failure.
  bool AddNameToStream(llvm::raw_string_ostream &Ostream,
                       const clang::NamedDecl *ND);

  MaybeFew<GraphObserver::NodeId> ApplyBuiltinTypeConstructor(
      const char *BuiltinName, const MaybeFew<GraphObserver::NodeId> &Param);

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

  /// \brief Returns the parent of the given node, along with the index
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
  template <typename NodeT> IndexedParent *getIndexedParent(const NodeT &Node) {
    return getIndexedParent(clang::ast_type_traits::DynTypedNode::create(Node));
  }

  /// \return true if `Decl` and all of the nodes underneath it are prunable.
  ///
  /// A subtree is prunable if it's "the same" in all possible indexer runs.
  /// This excludes, for example, certain template instantiations.
  bool declDominatesPrunableSubtree(const clang::Decl *Decl);

  IndexedParent *getIndexedParent(
      const clang::ast_type_traits::DynTypedNode &Node);
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

  /// Records information about the generic class by wrapping the node
  /// `BodyId`. Returns the `NodeId` for the dominating generic type.
  GraphObserver::NodeId RecordGenericClass(
      const clang::ObjCInterfaceDecl *IDecl,
      const clang::ObjCTypeParamList *TPL, const GraphObserver::NodeId &BodyId);

  /// \brief Fills `ArgNodeIds` with pointers to `NodeId`s for a template
  /// argument list.
  /// \param `ArgsAsWritten` Use this argument list if it isn't None.
  /// \param `Args` Use this argument list if `ArgsAsWritten` is None.
  /// \param `ArgIds` Vector into which pointers in `ArgNodeIds` will point.
  /// \param `ArgNodeIds` Vector of pointers to the result type list.
  /// \return true on success; if false, `ArgNodeIds` is invalid.
  bool BuildTemplateArgumentList(
      llvm::Optional<llvm::ArrayRef<clang::TemplateArgumentLoc>> ArgsAsWritten,
      llvm::ArrayRef<clang::TemplateArgument> Args,
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
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForDeclContext(
      const clang::DeclContext *DC);

  /// Points at the tapp node for a DeclContext, if it's an implicit template
  /// instantiation. Otherwise behaves as `BuildNodeIdForDeclContext`.
  MaybeFew<GraphObserver::NodeId> BuildNodeIdForRefToDeclContext(
      const clang::DeclContext *DC);

  /// Avoid regenerating type node IDs and keep track of where we are while
  /// generating node IDs for recursive types. The key is opaque and
  /// makes sense only within the implementation of this class.
  std::unordered_map<int64_t, MaybeFew<GraphObserver::NodeId>> TypeNodes;

  /// \brief Visit a DeclRefExpr or a ObjCIvarRefExpr
  ///
  /// DeclRefExpr and ObjCIvarRefExpr are similar entities and can be processed
  /// in the same way but do not have a useful common ancestry.
  bool VisitDeclRefOrIvarRefExpr(const clang::Expr *Expr,
                                 const clang::NamedDecl *const FoundDecl,
                                 clang::SourceLocation SL);

  /// \brief Connect a NodeId to the super and implemented protocols for a
  /// ObjCInterfaceDecl.
  ///
  /// Helper method used to connect an interface to the super and protocols it
  /// implements. It is also used to connect the interface implementation to
  /// these nodes as well. In that case, the interface implementation node is
  /// passed in as the first argument and the interface decl is passed in as the
  /// second.
  ///
  /// \param BodyDeclNode The node to connect to the super and protocols for
  /// the interface. This may be a interface decl node or an interface impl
  /// node.
  /// \param IFace The interface decl to use to look up the super and
  //// protocols.
  void ConnectToSuperClassAndProtocols(const GraphObserver::NodeId BodyDeclNode,
                                       const clang::ObjCInterfaceDecl *IFace);
  void ConnectToProtocols(const GraphObserver::NodeId BodyDeclNode,
                          clang::ObjCProtocolList::loc_iterator locStart,
                          clang::ObjCProtocolList::loc_iterator locEnd,
                          clang::ObjCProtocolList::iterator itStart,
                          clang::ObjCProtocolList::iterator itEnd);

  /// \brief Connect a parameter to a function decl.
  ///
  /// For FunctionDecl and ObjCMethodDecl, this connects the parameters to the
  /// function/method decl.
  /// \param Decl This should be a FunctionDecl or ObjCMethodDecl.
  void ConnectParam(const clang::Decl *Decl,
                    const GraphObserver::NodeId &FuncNode,
                    bool IsFunctionDefinition, const unsigned int ParamNumber,
                    const clang::ParmVarDecl *Param, bool DeclIsImplicit);

  /// \brief Record the type node for this id usage. This should only be called
  /// if T->getInterface() returns null, which means T is of type id and not
  /// a "real" class.
  ///
  /// If this is a naked id (no protocols listed), this returns the node id
  /// for the builtin id node. Otherwise, this calls BuildNodeIdForDecl for
  /// each protocol and constructs a new tapp node for this TypeUnion.
  MaybeFew<GraphObserver::NodeId> RecordIdTypeNode(
      const clang::ObjCObjectType *T,
      llvm::ArrayRef<clang::SourceLocation> ProtocolLocs);

  /// \brief Draw the completes edge from a Decl to each of its redecls.
  void RecordCompletesForRedecls(const clang::Decl *Decl,
                                 const clang::SourceRange &NameRange,
                                 const GraphObserver::NodeId &DeclNode);

  /// \brief Draw an extends/category edge from the category to the class the
  /// category is extending.
  ///
  /// For example, @interface A (Cat) ... We draw an extends edge from the
  /// ObjCCategoryDecl for Cat to the ObjCInterfaceDecl for A.
  ///
  /// \param DeclNode The node for the category (impl or decl).
  /// \param IFace The class interface for class we are adding a category to.
  void ConnectCategoryToBaseClass(const GraphObserver::NodeId &DeclNode,
                                  const clang::ObjCInterfaceDecl *IFace);

  void LogErrorWithASTDump(const std::string &msg,
                           const clang::Decl *Decl) const;
  void LogErrorWithASTDump(const std::string &msg,
                           const clang::Expr *Expr) const;

  /// \brief Mark each node of `NNSL` as a reference.
  /// \param NNSL the nested name specifier to visit.
  void VisitNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc NNSL);

  /// \brief This is used to handle the visitation of a clang::TypedefDecl
  /// or a clang::TypeAliasDecl.
  bool VisitCTypedef(const clang::TypedefNameDecl *Decl);

  /// \brief Find the implementation for `MD`. If `MD` is a definition, `MD` is
  /// returned. Otherwise, the method tries to find the implementation by
  /// looking through the interface and its implementation. If a method
  /// implementation is found, it is returned otherwise `MD` is returned.
  const clang::ObjCMethodDecl *FindMethodDefn(
      const clang::ObjCMethodDecl *MD, const clang::ObjCInterfaceDecl *I);

  /// \brief Maps known Decls to their NodeIds.
  llvm::DenseMap<const clang::Decl *, GraphObserver::NodeId> DeclToNodeId;

  /// \brief Maps EnumDecls to semantic hashes.
  llvm::DenseMap<const clang::EnumDecl *, uint64_t> EnumToHash;

  /// \brief Enabled library-specific callbacks.
  const LibrarySupports &Supports;

  /// \brief The `Sema` instance to use.
  clang::Sema &Sema;

  /// \brief The cache to use to generate signatures.
  MarkedSourceCache MarkedSources;

  /// \return true if we should stop indexing.
  std::function<bool()> ShouldStopIndexing;

  /// \brief The active indexing job.
  std::unique_ptr<IndexJob> Job;

  /// \brief The controlling worklist.
  std::unique_ptr<IndexerWorklist> Worklist;

  /// \brief Comments we've already visited.
  std::unordered_set<const clang::RawComment *> VisitedComments;
};

/// \brief An `ASTConsumer` that passes events to a `GraphObserver`.
class IndexerASTConsumer : public clang::SemaConsumer {
 public:
  explicit IndexerASTConsumer(
      GraphObserver *GO, BehaviorOnUnimplemented B, BehaviorOnTemplates T,
      Verbosity V, const LibrarySupports &S,
      std::function<bool()> ShouldStopIndexing,
      std::function<std::unique_ptr<IndexerWorklist>(IndexerASTVisitor *)>
          CreateWorklist)
      : Observer(GO),
        IgnoreUnimplemented(B),
        TemplateMode(T),
        Verbosity(V),
        Supports(S),
        ShouldStopIndexing(std::move(ShouldStopIndexing)),
        CreateWorklist(std::move(CreateWorklist)) {}

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    CHECK(Sema != nullptr);
    IndexerASTVisitor Visitor(Context, IgnoreUnimplemented, TemplateMode,
                              Verbosity, Supports, *Sema, ShouldStopIndexing,
                              Observer);
    {
      ProfileBlock block(Observer->getProfilingCallback(), "traverse_tu");
      Visitor.Work(Context.getTranslationUnitDecl(), CreateWorklist(&Visitor));
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
  /// \return true if we should stop indexing.
  std::function<bool()> ShouldStopIndexing;
  /// \return a new worklist for the given visitor.
  std::function<std::unique_ptr<IndexerWorklist>(IndexerASTVisitor *)>
      CreateWorklist;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_INDEXER_AST_HOOKS_H_
