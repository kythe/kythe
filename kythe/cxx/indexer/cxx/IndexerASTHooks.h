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

// Defines AST visitors and consumers used by the indexer tool.

#ifndef KYTHE_CXX_INDEXER_CXX_INDEXER_AST_HOOKS_H_
#define KYTHE_CXX_INDEXER_CXX_INDEXER_AST_HOOKS_H_

#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "GraphObserver.h"
#include "IndexerLibrarySupport.h"
#include "absl/base/attributes.h"
#include "absl/container/flat_hash_map.h"
#include "absl/log/check.h"
#include "absl/memory/memory.h"
#include "absl/strings/string_view.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTTypeTraits.h"
#include "clang/Sema/SemaConsumer.h"
#include "indexed_parent_map.h"
#include "indexer_worklist.h"
#include "kythe/cxx/indexer/cxx/node_set.h"
#include "kythe/cxx/indexer/cxx/recursive_type_visitor.h"
#include "kythe/cxx/indexer/cxx/semantic_hash.h"
#include "marked_source.h"
#include "re2/re2.h"
#include "type_map.h"

namespace kythe {

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

/// \brief Specifies if the indexer should emit documentation nodes for comments
/// associated with forward declarations.
enum BehaviorOnFwdDeclComments : bool { Emit = true, Ignore = false };

/// \brief A byte range that links to some node.
struct MiniAnchor {
  size_t Begin;
  size_t End;
  GraphObserver::NodeId AnchoredTo;
};

/// Adds brackets to Text to define anchor locations (escaping existing ones)
/// and sorts Anchors such that the ith Anchor corresponds to the ith opening
/// bracket. Drops empty or negative-length spans.
void InsertAnchorMarks(std::string& Text, std::vector<MiniAnchor>& Anchors);

/// \brief Used internally to check whether parts of the AST can be ignored.
class PruneCheck;

/// \brief Options that control how the indexer behaves.
struct IndexerOptions {
  /// \brief The directory to normalize paths against. Must be absolute.
  std::string EffectiveWorkingDirectory = "/";
  /// \brief Whether to allow access to the raw filesystem.
  bool AllowFSAccess = false;
  /// \brief Whether to drop data found to be template instantiation
  /// independent.
  bool DropInstantiationIndependentData = false;
  /// \brief A function that is called as the indexer enters and exits various
  /// phases of execution (in strict LIFO order).
  ProfilingCallback ReportProfileEvent = [](const char*, ProfilingEvent) {};
  /// \brief Whether to use the CompilationUnit VName corpus as the default
  /// corpus.
  bool UseCompilationCorpusAsDefault = false;
  /// \brief Whether we should stop on missing cases or continue on.
  BehaviorOnUnimplemented IgnoreUnimplemented = BehaviorOnUnimplemented::Abort;
  /// \brief Whether we should visit template instantiations.
  BehaviorOnTemplates TemplateMode = BehaviorOnTemplates::VisitInstantiations;
  /// \brief Should we emit documentation for forward class decls in ObjC?
  BehaviorOnFwdDeclComments ObjCFwdDocs = BehaviorOnFwdDeclComments::Emit;
  /// \brief Should we emit documentation for forward decls in C++?
  BehaviorOnFwdDeclComments CppFwdDocs = BehaviorOnFwdDeclComments::Emit;
  /// \return true if we should stop indexing.
  std::function<bool()> ShouldStopIndexing = [] { return false; };
  /// \brief The number of (raw) bytes to use to represent a USR. If 0,
  /// no USRs will be recorded.
  int UsrByteSize = 0;
  /// \brief Whether to use the default corpus when emitting USRs.
  bool EmitUsrCorpus = false;
  /// \brief if nonempty, the pattern to match a path against to see whether
  /// it should be excluded from template instance indexing.
  std::shared_ptr<re2::RE2> TemplateInstanceExcludePathPattern = nullptr;
  /// \param Worklist A function that generates a new worklist for the given
  /// visitor.
  std::function<std::unique_ptr<IndexerWorklist>(IndexerASTVisitor*)>
      CreateWorklist = [](IndexerASTVisitor* indexer) {
        return IndexerWorklist::CreateDefaultWorklist(indexer);
      };
  /// \brief Notified each time a semantic signature is hashed.
  HashRecorder* HashRecorder = nullptr;
  /// \brief If true, record directness of calls.
  bool RecordCallDirectness = false;
};

/// \brief An AST visitor that extracts information for a translation unit and
/// writes it to a `GraphObserver`.
class IndexerASTVisitor : public RecursiveTypeVisitor<IndexerASTVisitor> {
  using Base = RecursiveTypeVisitor;

 public:
  IndexerASTVisitor(clang::ASTContext& C, const LibrarySupports& S,
                    clang::Sema& Sema,
                    const IndexerOptions& IO ABSL_ATTRIBUTE_LIFETIME_BOUND,
                    GraphObserver* GO = nullptr)
      : Observer(GO ? *GO : NullObserver),
        Context(C),
        Supports(S),
        Sema(Sema),
        MarkedSources(&Sema, &Observer),
        options_(IO),
        Hash(
            [this](const clang::Decl* Decl) {
              return BuildNameIdForDecl(Decl).ToString();
            },
            // These enums are intentionally compatible.
            static_cast<SemanticHash::OnUnimplemented>(
                options_.IgnoreUnimplemented)) {}

  bool VisitDecl(const clang::Decl* Decl);
  bool TraverseFieldDecl(clang::FieldDecl* Decl);
  bool VisitFieldDecl(const clang::FieldDecl* Decl);
  bool TraverseVarDecl(clang::VarDecl* Decl);
  bool VisitVarDecl(const clang::VarDecl* Decl);
  bool VisitNamespaceDecl(const clang::NamespaceDecl* Decl);
  bool VisitBindingDecl(const clang::BindingDecl* Decl);
  bool VisitSizeOfPackExpr(const clang::SizeOfPackExpr* Expr);
  bool VisitDeclRefExpr(const clang::DeclRefExpr* DRE);

  bool TraverseCallExpr(clang::CallExpr* CE);
  bool TraverseReturnStmt(clang::ReturnStmt* RS);
  bool TraverseBinaryOperator(clang::BinaryOperator* BO);
  bool TraverseUnaryOperator(clang::UnaryOperator* BO);
  bool TraverseCompoundAssignOperator(clang::CompoundAssignOperator* CAO);

  bool TraverseInitListExpr(clang::InitListExpr* ILE);
  bool VisitInitListExpr(const clang::InitListExpr* ILE);
  bool VisitDesignatedInitExpr(const clang::DesignatedInitExpr* DIE);

  bool VisitCXXConstructExpr(const clang::CXXConstructExpr* E);
  bool VisitCXXDeleteExpr(const clang::CXXDeleteExpr* E);
  bool VisitCXXNewExpr(const clang::CXXNewExpr* E);
  bool VisitCXXPseudoDestructorExpr(const clang::CXXPseudoDestructorExpr* E);
  bool VisitCXXUnresolvedConstructExpr(
      const clang::CXXUnresolvedConstructExpr* E);
  bool TraverseCXXOperatorCallExpr(clang::CXXOperatorCallExpr* E);
  bool VisitCallExpr(const clang::CallExpr* Expr);
  bool VisitMemberExpr(const clang::MemberExpr* Expr);
  bool VisitCXXDependentScopeMemberExpr(
      const clang::CXXDependentScopeMemberExpr* Expr);

  bool TraverseNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc NNS);

  // Visitors for leaf TypeLoc types.
  bool VisitBuiltinTypeLoc(clang::BuiltinTypeLoc TL);
  bool VisitEnumTypeLoc(clang::EnumTypeLoc TL);
  bool VisitRecordTypeLoc(clang::RecordTypeLoc TL);
  bool VisitTemplateTypeParmTypeLoc(clang::TemplateTypeParmTypeLoc TL);
  bool VisitSubstTemplateTypeParmTypeLoc(
      clang::SubstTemplateTypeParmTypeLoc TL);

  template <typename TypeLoc, typename Type>
  bool VisitTemplateSpecializationTypePairHelper(TypeLoc Written,
                                                 const Type* Resolved);

  bool VisitTemplateSpecializationTypeLoc(
      clang::TemplateSpecializationTypeLoc TL);
  bool VisitDeducedTemplateSpecializationTypePair(
      clang::DeducedTemplateSpecializationTypeLoc TL,
      const clang::DeducedTemplateSpecializationType* T);

  bool VisitAutoTypePair(clang::AutoTypeLoc TL, const clang::AutoType* T);
  bool VisitDecltypeTypeLoc(clang::DecltypeTypeLoc TL);
  bool VisitElaboratedTypeLoc(clang::ElaboratedTypeLoc TL);
  bool VisitTypedefTypeLoc(clang::TypedefTypeLoc TL);
  bool VisitUsingTypeLoc(clang::UsingTypeLoc TL);
  bool VisitInjectedClassNameTypeLoc(clang::InjectedClassNameTypeLoc TL);
  bool VisitDependentNameTypeLoc(clang::DependentNameTypeLoc TL);
  bool VisitPackExpansionTypeLoc(clang::PackExpansionTypeLoc TL);
  bool VisitObjCObjectTypeLoc(clang::ObjCObjectTypeLoc TL);
  bool VisitObjCTypeParamTypeLoc(clang::ObjCTypeParamTypeLoc TL);

  bool TraverseAttributedTypeLoc(clang::AttributedTypeLoc TL);
  bool TraverseDependentAddressSpaceTypeLoc(
      clang::DependentAddressSpaceTypeLoc TL);

  bool TraverseMemberPointerTypeLoc(clang::MemberPointerTypeLoc TL);

  // Emit edges for an anchor pointing to the indicated type.
  NodeSet RecordTypeLocSpellingLocation(clang::TypeLoc TL);
  NodeSet RecordTypeLocSpellingLocation(clang::TypeLoc Written,
                                        const clang::Type* Resolved);
  NodeSet RecordTypeSpellingLocation(const clang::Type* Type,
                                     clang::SourceRange Range);

  bool TraverseDeclarationNameInfo(clang::DeclarationNameInfo NameInfo);

  // Visit the subtypes of TypedefNameDecl individually because we want to do
  // something different with ObjCTypeParamDecl.
  bool VisitTypedefDecl(const clang::TypedefDecl* Decl);
  bool VisitTypeAliasDecl(const clang::TypeAliasDecl* Decl);
  bool VisitObjCTypeParamDecl(const clang::ObjCTypeParamDecl* Decl);
  bool VisitUsingShadowDecl(const clang::UsingShadowDecl* Decl);

  bool VisitRecordDecl(const clang::RecordDecl* Decl);
  bool VisitEnumDecl(const clang::EnumDecl* Decl);
  bool VisitEnumConstantDecl(const clang::EnumConstantDecl* Decl);
  bool VisitFunctionDecl(clang::FunctionDecl* Decl);

  bool TraverseDecl(clang::Decl* Decl);
  bool TraverseCXXConstructorDecl(clang::CXXConstructorDecl* CD);
  bool TraverseCXXDefaultInitExpr(const clang::CXXDefaultInitExpr* E);

  bool TraverseConstructorInitializer(clang::CXXCtorInitializer* Init);
  bool TraverseCXXNewExpr(clang::CXXNewExpr* E);
  bool TraverseCXXFunctionalCastExpr(clang::CXXFunctionalCastExpr* E);
  bool TraverseCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr* E);

  bool IndexConstructExpr(const clang::CXXConstructExpr* E,
                          const clang::TypeSourceInfo* TSI);

  // Objective C specific nodes
  bool VisitObjCPropertyImplDecl(const clang::ObjCPropertyImplDecl* Decl);
  bool VisitObjCCompatibleAliasDecl(const clang::ObjCCompatibleAliasDecl* Decl);
  bool VisitObjCCategoryDecl(const clang::ObjCCategoryDecl* Decl);
  bool VisitObjCImplementationDecl(
      const clang::ObjCImplementationDecl* ImplDecl);
  bool VisitObjCCategoryImplDecl(const clang::ObjCCategoryImplDecl* ImplDecl);
  bool VisitObjCInterfaceDecl(const clang::ObjCInterfaceDecl* Decl);
  bool VisitObjCProtocolDecl(const clang::ObjCProtocolDecl* Decl);
  bool VisitObjCMethodDecl(const clang::ObjCMethodDecl* Decl);
  bool VisitObjCPropertyDecl(const clang::ObjCPropertyDecl* Decl);
  bool VisitObjCIvarRefExpr(const clang::ObjCIvarRefExpr* IRE);
  bool VisitObjCMessageExpr(const clang::ObjCMessageExpr* Expr);
  bool VisitObjCPropertyRefExpr(const clang::ObjCPropertyRefExpr* Expr);

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

  // Objective C methods don't have TypeSourceInfo so we must construct a type
  // for the methods to be used in the graph.
  std::optional<GraphObserver::NodeId> CreateObjCMethodTypeNode(
      const clang::ObjCMethodDecl* MD);

  /// \brief Builds a stable node ID for a compile-time expression.
  /// \param Expr The expression to represent.
  std::optional<GraphObserver::NodeId> BuildNodeIdForExpr(
      const clang::Expr* Expr);

  /// \brief Builds a stable node ID for a special template argument.
  /// \param Id A string representing the special argument.
  GraphObserver::NodeId BuildNodeIdForSpecialTemplateArgument(
      llvm::StringRef Id);

  /// \brief Builds a stable node ID for a template expansion template argument.
  /// \param Name The template pattern being expanded.
  std::optional<GraphObserver::NodeId> BuildNodeIdForTemplateExpansion(
      clang::TemplateName Name);

  /// \brief Builds a stable NodeSet for the given TypeLoc.
  /// \param TL The TypeLoc for which to build a NodeSet.
  /// \returns NodeSet instance indicating claimability of the contained
  /// NodeIds.
  NodeSet BuildNodeSetForType(const clang::QualType& QT);
  NodeSet BuildNodeSetForType(const clang::Type* T);
  NodeSet BuildNodeSetForType(const clang::TypeLoc& TL);

  NodeSet BuildNodeSetForTypeInternal(const clang::Type& T);
  NodeSet BuildNodeSetForTypeInternal(const clang::QualType& QT);

  NodeSet BuildNodeSetForBuiltin(const clang::BuiltinType& T) const;
  NodeSet BuildNodeSetForEnum(const clang::EnumType& T);
  NodeSet BuildNodeSetForRecord(const clang::RecordType& T);
  NodeSet BuildNodeSetForInjectedClassName(
      const clang::InjectedClassNameType& T);
  NodeSet BuildNodeSetForTemplateTypeParm(const clang::TemplateTypeParmType& T);
  NodeSet BuildNodeSetForPointer(const clang::PointerType& T);
  NodeSet BuildNodeSetForMemberPointer(const clang::MemberPointerType& T);
  NodeSet BuildNodeSetForLValueReference(const clang::LValueReferenceType& T);
  NodeSet BuildNodeSetForRValueReference(const clang::RValueReferenceType& T);

  NodeSet BuildNodeSetForAuto(const clang::AutoType& TL);
  NodeSet BuildNodeSetForDeducedTemplateSpecialization(
      const clang::DeducedTemplateSpecializationType& TL);
  // Helper used for Auto and DeducedTemplateSpecialization.
  NodeSet BuildNodeSetForDeduced(const clang::DeducedType& T);

  NodeSet BuildNodeSetForConstantArray(const clang::ConstantArrayType& TL);
  NodeSet BuildNodeSetForIncompleteArray(const clang::IncompleteArrayType& TL);
  NodeSet BuildNodeSetForDependentSizedArray(
      const clang::DependentSizedArrayType& T);
  NodeSet BuildNodeSetForBitInt(const clang::BitIntType& T);
  NodeSet BuildNodeSetForDependentBitInt(const clang::DependentBitIntType& T);
  NodeSet BuildNodeSetForFunctionProto(const clang::FunctionProtoType& T);
  NodeSet BuildNodeSetForFunctionNoProto(const clang::FunctionNoProtoType& T);
  NodeSet BuildNodeSetForParen(const clang::ParenType& T);
  NodeSet BuildNodeSetForDecltype(const clang::DecltypeType& T);
  NodeSet BuildNodeSetForElaborated(const clang::ElaboratedType& T);
  NodeSet BuildNodeSetForTypedef(const clang::TypedefType& T);
  NodeSet BuildNodeSetForUsing(const clang::UsingType& T);

  NodeSet BuildNodeSetForSubstTemplateTypeParm(
      const clang::SubstTemplateTypeParmType& T);
  NodeSet BuildNodeSetForDependentName(const clang::DependentNameType& T);
  NodeSet BuildNodeSetForTemplateSpecialization(
      const clang::TemplateSpecializationType& T);
  NodeSet BuildNodeSetForPackExpansion(const clang::PackExpansionType& T);
  NodeSet BuildNodeSetForBlockPointer(const clang::BlockPointerType& T);
  NodeSet BuildNodeSetForObjCObjectPointer(
      const clang::ObjCObjectPointerType& T);
  NodeSet BuildNodeSetForObjCObject(const clang::ObjCObjectType& T);
  NodeSet BuildNodeSetForObjCTypeParam(const clang::ObjCTypeParamType& T);
  NodeSet BuildNodeSetForObjCInterface(const clang::ObjCInterfaceType& T);
  NodeSet BuildNodeSetForAttributed(const clang::AttributedType& T);
  NodeSet BuildNodeSetForDependentAddressSpace(
      const clang::DependentAddressSpaceType& T);

  // Helper function which constructs marked source and records
  // a tnominal node for the given `Decl`.
  GraphObserver::NodeId BuildNominalNodeIdForDecl(const clang::NamedDecl* Decl);

  // Helper used by BuildNodeSetForRecord and BuildNodeSetForInjectedClassName.
  NodeSet BuildNodeSetForNonSpecializedRecordDecl(
      const clang::RecordDecl* Decl);

  const clang::TemplateTypeParmDecl* FindTemplateTypeParmTypeDecl(
      const clang::TemplateTypeParmType& T) const;

  std::optional<GraphObserver::NodeId> BuildNodeIdForObjCProtocols(
      const clang::ObjCObjectType& T);
  GraphObserver::NodeId BuildNodeIdForObjCProtocols(
      absl::Span<const GraphObserver::NodeId> ProtocolIds);

  std::vector<GraphObserver::NodeId> BuildNodeIdsForObjCProtocols(
      GraphObserver::NodeId BaseType, const clang::ObjCObjectType& T);
  std::vector<GraphObserver::NodeId> BuildNodeIdsForObjCProtocols(
      const clang::ObjCObjectType& T);

  /// \brief Builds a stable node ID for `Type`.
  /// \param TypeLoc The type that is being identified.
  /// \return The Node ID for `Type`.
  std::optional<GraphObserver::NodeId> BuildNodeIdForType(
      const clang::TypeLoc& TypeLoc);

  /// \brief Builds a stable node ID for `QT`.
  /// \param QT The type that is being identified.
  /// \return The Node ID for `QT`.
  std::optional<GraphObserver::NodeId> BuildNodeIdForType(
      const clang::QualType& QT);

  /// \brief Builds a stable node ID for `T`.
  /// \param T The type that is being identified.
  /// \return The Node ID for `T`.
  std::optional<GraphObserver::NodeId> BuildNodeIdForType(const clang::Type* T);

  /// \brief Builds a stable node ID for the given `TemplateName`.
  std::optional<GraphObserver::NodeId> BuildNodeIdForTemplateName(
      const clang::TemplateName& Name);

  /// \brief Builds a stable node ID for the given `TemplateArgumentLoc`.
  std::optional<GraphObserver::NodeId> BuildNodeIdForTemplateArgument(
      const clang::TemplateArgumentLoc& ArgLoc);

  /// \brief Builds a stable node ID for the given `TemplateArgument`.
  std::optional<GraphObserver::NodeId> BuildNodeIdForTemplateArgument(
      const clang::TemplateArgument& Arg);

  /// \brief Builds a stable node ID for `Stmt`.
  ///
  /// This mechanism for generating IDs should only be used in contexts where
  /// identifying statements through source locations/wraith contexts is not
  /// possible (e.g., in implicit code).
  ///
  /// \param Decl The statement that is being identified
  /// \return The node for `Stmt` if the statement was implicit; otherwise,
  /// None.
  std::optional<GraphObserver::NodeId> BuildNodeIdForImplicitStmt(
      const clang::Stmt* Stmt);

  /// \brief Builds a stable node ID for `Decl`'s tapp if it's an implicit
  /// template instantiation.
  std::optional<GraphObserver::NodeId>
  BuildNodeIdForImplicitTemplateInstantiation(const clang::Decl* Decl);

  /// \brief Builds a stable node ID for `Decl`'s tapp if it's an implicit
  /// function template instantiation.
  std::optional<GraphObserver::NodeId>
  BuildNodeIdForImplicitFunctionTemplateInstantiation(
      const clang::FunctionDecl* FD);

  /// \brief Builds a stable node ID for `Decl`'s tapp if it's an implicit
  /// class template instantiation.
  std::optional<GraphObserver::NodeId>
  BuildNodeIdForImplicitClassTemplateInstantiation(
      const clang::ClassTemplateSpecializationDecl* CTSD);

  /// \brief Builds a stable node ID for an external reference to `Decl`.
  ///
  /// This is equivalent to BuildNodeIdForDecl for Decls that are not
  /// implicit template instantiations; otherwise, it returns the `NodeId`
  /// for the tapp node for the instantiation.
  ///
  /// \param Decl The declaration that is being identified.
  /// \return The node for `Decl`.
  GraphObserver::NodeId BuildNodeIdForRefToDecl(const clang::Decl* Decl);

  /// \brief Builds a stable node ID for `Decl`.
  ///
  /// \param Decl The declaration that is being identified.
  /// \return The node for `Decl`.
  GraphObserver::NodeId BuildNodeIdForDecl(const clang::Decl* Decl);

  /// \brief Builds a stable node ID for the definition of `Decl`, if
  /// `Decl` is not already a definition and its definition can be found.
  ///
  /// \param Decl The declaration that is being identified.
  /// \return The node for the definition `Decl` if `Decl` isn't a definition
  /// and its definition can be found; or None.
  template <typename D>
  std::optional<GraphObserver::NodeId> BuildNodeIdForDefnOfDecl(const D* Decl) {
    if (const auto* Defn = Decl->getDefinition()) {
      if (Defn != Decl) {
        return BuildNodeIdForDecl(Defn);
      }
    }
    return std::nullopt;
  }

  /// \brief Builds a stable name ID for the name of `Decl`.
  ///
  /// \param Decl The declaration that is being named.
  /// \return The name for `Decl`.
  GraphObserver::NameId BuildNameIdForDecl(const clang::Decl* Decl);

  /// \brief Records parameter and lookup edges for the given dependent name.
  ///
  /// \param NNS The qualifier on the name.
  /// \param Id The name itself.
  /// \param IdLoc The name's location.
  /// \param Root If provided, the primary NodeId is morally prepended to `NNS`
  /// such that the dependent name is lookup(lookup*(Root, NNS), Id).
  /// \return The NodeId for the dependent name.
  std::optional<GraphObserver::NodeId> RecordEdgesForDependentName(
      const clang::NestedNameSpecifierLoc& NNS,
      const clang::DeclarationName& Id, const clang::SourceLocation IdLoc,
      const std::optional<GraphObserver::NodeId>& Root = std::nullopt);

  /// \brief Records parameter edges for the given dependent name.
  /// Also records Lookup edges for any nested Identifiers.
  /// \param DId The NodeId of the dependent name to use.
  /// \param NNSLoc The qualifier prefix of the dependent name, if any.
  /// \param Root If provided, the primary NodeId to prepend to `NNS`.
  /// \return The provided NodeId or nullopt if an unhandled element is
  /// encountered.
  std::optional<GraphObserver::NodeId> RecordParamEdgesForDependentName(
      const GraphObserver::NodeId& DId, clang::NestedNameSpecifierLoc NNSLoc,
      const std::optional<GraphObserver::NodeId>& Root = std::nullopt);

  /// \brief Records the lookup edge for a dependent name.
  ///
  /// \param DId The NodeId of the name being looked up.
  /// \param Name The kind of name being looked up.
  /// \return The provided NodeId or std::nullopt if Name is unsupported.
  std::optional<GraphObserver::NodeId> RecordLookupEdgeForDependentName(
      const GraphObserver::NodeId& DId, const clang::DeclarationName& Name);

  /// \brief Builds a NodeId for the DependentName.
  ///
  /// \param Prefix The qualifier preceding the name.
  /// \param Identifier The identifier in question.
  GraphObserver::NodeId BuildNodeIdForDependentIdentifier(
      const clang::NestedNameSpecifier* Prefix,
      const clang::IdentifierInfo* Identifier);

  /// \brief Builds a NodeId for the DependentName.
  ///
  /// \param Prefix The qualifier preceding the name.
  /// \param Identifier The DeclarationName in question.
  GraphObserver::NodeId BuildNodeIdForDependentName(
      const clang::NestedNameSpecifier* Prefix,
      const clang::DeclarationName& Identifier);

  /// \brief Builds a NodeId for the provided NestedNameSpecifier, depending on
  /// its type.
  ///
  /// \param NNSLoc The NestedNameSpecifierLoc from which to construct a NodeId.
  std::optional<GraphObserver::NodeId> BuildNodeIdForNestedNameSpecifier(
      const clang::NestedNameSpecifier* NNS);

  /// \brief Builds a NodeId for the provided NestedNameSpecifier, depending on
  /// its type.
  ///
  /// \param NNSLoc The NestedNameSpecifierLoc from which to construct a NodeId.
  std::optional<GraphObserver::NodeId> BuildNodeIdForNestedNameSpecifierLoc(
      const clang::NestedNameSpecifierLoc& NNSLoc);

  /// \brief Is `VarDecl` a definition?
  ///
  /// A parameter declaration is considered a definition if it appears as part
  /// of a function definition; otherwise it is always a declaration. This
  /// differs from the C++ Standard, which treats these parameters as
  /// definitions (basic.scope.proto).
  static bool IsDefinition(const clang::VarDecl* VD);

  /// \brief Is `FunctionDecl` a definition?
  static bool IsDefinition(const clang::FunctionDecl* FunctionDecl);

  /// \brief Gets a range for the name of a declaration, whether that name is a
  /// single token or otherwise.
  ///
  /// The returned range is a best-effort attempt to cover the "name" of
  /// the entity as written in the source code.
  clang::SourceRange RangeForNameOfDeclaration(
      const clang::NamedDecl* Decl) const;

  bool TraverseClassTemplateDecl(clang::ClassTemplateDecl* TD);
  bool TraverseClassTemplateSpecializationDecl(
      clang::ClassTemplateSpecializationDecl* TD);
  bool TraverseClassTemplatePartialSpecializationDecl(
      clang::ClassTemplatePartialSpecializationDecl* TD);

  bool TraverseVarTemplateDecl(clang::VarTemplateDecl* TD);
  bool TraverseVarTemplateSpecializationDecl(
      clang::VarTemplateSpecializationDecl* TD);
  bool ForceTraverseVarTemplateSpecializationDecl(
      clang::VarTemplateSpecializationDecl* TD);
  bool TraverseVarTemplatePartialSpecializationDecl(
      clang::VarTemplatePartialSpecializationDecl* TD);

  bool TraverseFunctionDecl(clang::FunctionDecl* FD);
  bool TraverseFunctionTemplateDecl(clang::FunctionTemplateDecl* FTD);

  bool TraverseTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl* TATD);

  bool shouldVisitTemplateInstantiations() const {
    return options_.TemplateMode == BehaviorOnTemplates::VisitInstantiations;
  }
  bool shouldEmitObjCForwardClassDeclDocumentation() const {
    return options_.ObjCFwdDocs == BehaviorOnFwdDeclComments::Emit;
  }
  bool shouldEmitCppForwardDeclDocumentation() const {
    return options_.CppFwdDocs == BehaviorOnFwdDeclComments::Emit;
  }
  bool shouldVisitImplicitCode() const { return true; }
  // Disables data recursion. We intercept Traverse* methods in the RAV, which
  // are not triggered during data recursion.
  bool shouldUseDataRecursionFor(clang::Stmt* S) const { return false; }

  /// \brief Records the range of a definition. If the range cannot be placed
  /// somewhere inside a source file, no record is made.
  void MaybeRecordDefinitionRange(
      const std::optional<GraphObserver::Range>& R,
      const GraphObserver::NodeId& Id,
      const std::optional<GraphObserver::NodeId>& DeclId);

  /// \brief Records the full range of a definition. If the range cannot be
  /// placed somewhere inside a source file, no record is made.
  void MaybeRecordFullDefinitionRange(
      const std::optional<GraphObserver::Range>& R,
      const GraphObserver::NodeId& Id,
      const std::optional<GraphObserver::NodeId>& DeclId);

  /// \brief Returns the attached GraphObserver.
  GraphObserver& getGraphObserver() { return Observer; }

  /// \brief Returns the ASTContext.
  const clang::ASTContext& getASTContext() { return Context; }

  /// Returns `SR` as a `Range` in this `RecursiveASTVisitor`'s current
  /// RangeContext.
  std::optional<GraphObserver::Range> ExplicitRangeInCurrentContext(
      const clang::SourceRange& SR);

  /// Returns `SR` as a `Range` in this `RecursiveASTVisitor`'s current
  /// RangeContext. If SR is in a macro, the returned Range will be mapped
  /// to a file first. If the range would be zero-width, it will be expanded
  /// via RangeForASTEntityFromSourceLocation.
  std::optional<GraphObserver::Range> ExpandedRangeInCurrentContext(
      clang::SourceRange SR);
  /// Returns `SR` as a character-based file range.
  clang::SourceRange NormalizeRange(clang::SourceRange SR) const;

  /// If `Implicit` is true, returns `Id` as an implicit Range; otherwise,
  /// returns `SR` as a `Range` in this `RecursiveASTVisitor`'s current
  /// RangeContext.
  std::optional<GraphObserver::Range> RangeInCurrentContext(
      bool Implicit, const GraphObserver::NodeId& Id,
      const clang::SourceRange& SR);

  /// If `Id` is some NodeId, returns it as an implicit Range; otherwise,
  /// returns `SR` as a `Range` in this `RecursiveASTVisitor`'s current
  /// RangeContext.
  std::optional<GraphObserver::Range> RangeInCurrentContext(
      const std::optional<GraphObserver::NodeId>& Id,
      const clang::SourceRange& SR);

  void RunJob(std::unique_ptr<IndexJob> JobToRun) {
    Job = std::move(JobToRun);
    if (Job->IsDeclJob) {
      TraverseDecl(Job->Decl);
    } else {
      // There is no declaration attached to a top-level file comment.
      HandleFileLevelComments(Job->FileId, Job->FileNode);
    }
  }

  const IndexJob& getCurrentJob() {
    CHECK(Job != nullptr);
    return *Job;
  }

  /// \brief Call after sema but before traversal. Applies semantic metadata
  /// (e.g., write tags, alias tags).
  void PrepareAlternateSemanticCache();

  void Work(clang::Decl* InitialDecl,
            std::unique_ptr<IndexerWorklist> NewWorklist) {
    Worklist = std::move(NewWorklist);
    Worklist->EnqueueJob(std::make_unique<IndexJob>(InitialDecl));
    while (!options_.ShouldStopIndexing() && Worklist->DoWork())
      ;
    Observer.iterateOverClaimedFiles(
        [this, InitialDecl](clang::FileID Id,
                            const GraphObserver::NodeId& FileNode) {
          RunJob(std::make_unique<IndexJob>(InitialDecl, Id, FileNode));
          return !options_.ShouldStopIndexing();
        });
    Worklist.reset();
  }

  /// \brief Provides execute-only access to ShouldStopIndexing. Should be used
  /// from the same thread that's walking the AST.
  bool shouldStopIndexing() const { return options_.ShouldStopIndexing(); }

  /// Blames a call to `Callee` at `Range` on everything at the top of
  /// `BlameStack` (or does nothing if there's nobody to blame).
  void RecordCallEdges(
      const GraphObserver::Range& Range, const GraphObserver::NodeId& Callee,
      GraphObserver::CallDispatch D = GraphObserver::CallDispatch::kDefault);

  // Blames the use of a `Decl` at a particular `Range` on everything at the
  // top of `BlameStack`. If there is nothing at the top of `BlameStack`,
  // blames the use on the file.
  void RecordBlame(const clang::Decl* Decl, const GraphObserver::Range& Range);

  /// \return whether `range` should be considered to be implicit under the
  /// current context.
  GraphObserver::Implicit IsImplicit(const GraphObserver::Range& range);

 private:
  friend class PruneCheck;

  NullGraphObserver NullObserver;
  GraphObserver& Observer;
  clang::ASTContext& Context;

  /// \return the `Decl` that is the target of influence by an lexpression with
  /// head `head`, or null. For example, in `foo[x].bar(y).z`, the target of
  /// influence is the member decl for `z`.
  const clang::Decl* GetInfluencedDeclFromLValueHead(const clang::Expr* head);

  /// \brief Describes a target (possibly sub-)expression resulting from some
  /// operation.
  ///
  /// Often parts of an expression are especially interesting, like
  /// lvalue heads (`z` in `x.y.z`) or the results of eliminating simple aliases
  /// (`x` in `*&x`). The `head` field will contain that subexpression. If
  /// the entire expression is relevant, or if no subexpression could be found,
  /// `head` will contain the input expression.
  ///
  /// It may also be the case that the indexer has more information about
  /// a particular target, e.g. that it should semantically refer to some
  /// remote node rather than the one indicated by the (sub)expression. In this
  /// case the `alt` field will contain that remote node's ID; this should be
  /// preferred to `head`. For example, `alt` will be set to the ID of the
  /// `foo` field in `*x->mutable_foo()`.
  struct TargetExpr {
    const clang::Expr* head;                   ///< The target (sub)expression.
    std::optional<GraphObserver::NodeId> alt;  ///< The preferred target.
  };

  /// \brief Eliminates the trivial introduction of aliasing.
  ///
  /// In some situations (and modulo undefined behavior), the expression
  /// `*(&foo)` can be reduced to `foo`. `foo` may be a simple DeclRefExpr, but
  /// it could also be a MemberExpr that has alias semantics tied to another
  /// field or some piece of generated code.
  ///
  /// \return a TargetExpr with the alias eliminated, or with the original
  /// `expr` set if no alias could be eliminated.
  TargetExpr SkipTrivialAliasing(const clang::Expr* expr);

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
                          clang::Token& Token) const {
    return Observer.getPreprocessor()->getRawToken(StartLocation, Token,
                                                   true /* ignoreWhiteSpace */)
               ? LexerResult::Failure
               : LexerResult::Success;
  }

  /// \brief Handle the file-level comments for `Id` with node ID `FileId`.
  void HandleFileLevelComments(clang::FileID Id,
                               const GraphObserver::NodeId& FileNode);

  /// \brief Emit data for `Comment` that documents `DocumentedNode`, using
  /// `DC` for lookups.
  void VisitComment(const clang::RawComment* Comment,
                    const clang::DeclContext* DC,
                    const GraphObserver::NodeId& DocumentedNode);

  /// \brief Emit data for attributes attached to `Decl`, whose `NodeId`
  /// is `TargetNode`.
  void VisitAttributes(const clang::Decl* Decl,
                       const GraphObserver::NodeId& TargetNode);

  /// \brief Attempts to find the ID of the first parent of `Decl` for
  /// attaching a `childof` relationship.
  std::optional<GraphObserver::NodeId> GetDeclChildOf(const clang::Decl* D);

  /// \brief Assign `ND` (whose node ID is `TargetNode`) a USR if USRs are
  /// enabled.
  ///
  /// USRs are added only for NamedDecls that:
  ///   * are not under implicit template instantiations
  ///   * are not in a DeclContext inside a function body
  ///   * can actually be assigned USRs from Clang
  ///
  /// Similar to the way we deal with JVM names, the corpus, path,
  /// and root fields of a usr vname are cleared. Clients are permitted
  /// to write their own USR tickets. The USR value itself is encoded
  /// in capital hex (to match Clang's own internal USR stringification,
  /// modulo the configurable size of the SHA1 prefix).
  void AssignUSR(const GraphObserver::NodeId& TargetNode,
                 const clang::NamedDecl* ND);

  /// Assigns a USR to an alias.
  void AssignUSR(const GraphObserver::NameId& TargetName,
                 const GraphObserver::NodeId& AliasedType,
                 const clang::NamedDecl* ND) {
    AssignUSR(Observer.nodeIdForTypeAliasNode(TargetName, AliasedType), ND);
  }

  GraphObserver::NodeId ApplyBuiltinTypeConstructor(
      const char* BuiltinName, const GraphObserver::NodeId& Param);

  /// \return true if `Decl` and all of the nodes underneath it are prunable.
  ///
  /// A subtree is prunable if it's "the same" in all possible indexer runs.
  /// This excludes, for example, certain template instantiations.
  bool declDominatesPrunableSubtree(const clang::Decl* Decl);

  /// Initializes AllParents, if necessary, and then returns a pointer to it.
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
  const IndexedParentMap* getAllParents() const;

  // Used for building the indexed parent map and recording the time it took.
  IndexedParentMap BuildIndexedParentMap() const;
  /// A map from memoizable DynTypedNodes to their parent nodes
  /// and their child indices with respect to those parents.
  /// Filled on the first call to `getIndexedParents`.
  LazyIndexedParentMap AllParents{[this] { return BuildIndexedParentMap(); }};

  /// Records information about the template `Template` for the node
  /// `DeclNode`, including the edges linking the template with its parameters.
  template <typename TemplateDeclish>
  void RecordTemplate(const TemplateDeclish* Decl,
                      const GraphObserver::NodeId& DeclNode);

  /// Records information about the generic class by wrapping the node
  /// `BodyId`. Returns the `NodeId` for the dominating generic type.
  GraphObserver::NodeId RecordGenericClass(
      const clang::ObjCInterfaceDecl* IDecl,
      const clang::ObjCTypeParamList* TPL, const GraphObserver::NodeId& BodyId);

  /// \brief Returns a vector of NodeId for each template argument.
  std::optional<std::vector<GraphObserver::NodeId>> BuildTemplateArgumentList(
      llvm::ArrayRef<clang::TemplateArgument> Args);
  std::optional<std::vector<GraphObserver::NodeId>> BuildTemplateArgumentList(
      llvm::ArrayRef<clang::TemplateArgumentLoc> Args);

  /// Dumps information about `TypeContext` to standard error when looking for
  /// an entry at (`Depth`, `Index`).
  void DumpTypeContext(unsigned Depth, unsigned Index);

  /// \brief Attempts to add a childof edge between DeclNode and its parent.
  /// \param Decl The (outer, in the case of a template) decl.
  /// \param DeclNode The (outer) `NodeId` for `Decl`.
  void AddChildOfEdgeToDeclContext(const clang::Decl* Decl,
                                   const GraphObserver::NodeId& DeclNode);

  /// Points at the inner node of the DeclContext, if it's a template.
  /// Otherwise points at the DeclContext as a Decl.
  std::optional<GraphObserver::NodeId> BuildNodeIdForDeclContext(
      const clang::DeclContext* DC);

  /// Points at the tapp node for a DeclContext, if it's an implicit template
  /// instantiation. Otherwise behaves as `BuildNodeIdForDeclContext`.
  std::optional<GraphObserver::NodeId> BuildNodeIdForRefToDeclContext(
      const clang::DeclContext* DC);

  /// Avoid regenerating type node IDs and keep track of where we are while
  /// generating node IDs for recursive types. The key is opaque and
  /// makes sense only within the implementation of this class.
  TypeMap<NodeSet> TypeNodes;

  /// \brief Visit an Expr that refers to some NamedDecl.
  ///
  /// DeclRefExpr and ObjCIvarRefExpr are similar entities and can be processed
  /// in the same way but do not have a useful common ancestry.
  bool VisitDeclRefOrIvarRefExpr(const clang::Expr* Expr,
                                 const clang::NamedDecl* const FoundDecl,
                                 clang::SourceLocation SL,
                                 bool IsImplicit = false);

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
                                       const clang::ObjCInterfaceDecl* IFace);
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
  void ConnectParam(const clang::Decl* Decl,
                    const GraphObserver::NodeId& FuncNode,
                    bool IsFunctionDefinition, const unsigned int ParamNumber,
                    const clang::ParmVarDecl* Param, bool DeclIsImplicit);

  /// \brief Draw the completes edge from a Decl to each of its redecls.
  void RecordCompletesForRedecls(const clang::Decl* Decl,
                                 const clang::SourceRange& NameRange,
                                 const GraphObserver::NodeId& DeclNode);

  /// \brief Draw an extends/category edge from the category to the class the
  /// category is extending.
  ///
  /// For example, @interface A (Cat) ... We draw an extends edge from the
  /// ObjCCategoryDecl for Cat to the ObjCInterfaceDecl for A.
  ///
  /// \param DeclNode The node for the category (impl or decl).
  /// \param IFace The class interface for class we are adding a category to.
  void ConnectCategoryToBaseClass(const GraphObserver::NodeId& DeclNode,
                                  const clang::ObjCInterfaceDecl* IFace);

  void LogErrorWithASTDump(absl::string_view msg,
                           const clang::Decl* Decl) const;
  void LogErrorWithASTDump(absl::string_view msg,
                           const clang::Expr* Expr) const;
  void LogErrorWithASTDump(absl::string_view msg,
                           const clang::Type* Type) const;
  void LogErrorWithASTDump(absl::string_view msg, clang::TypeLoc Type) const;
  void LogErrorWithASTDump(absl::string_view msg, clang::QualType Type) const;

  /// \brief This is used to handle the visitation of a clang::TypedefDecl
  /// or a clang::TypeAliasDecl.
  bool VisitCTypedef(const clang::TypedefNameDecl* Decl);

  /// \brief Find the implementation for `MD`. If `MD` is a definition, `MD` is
  /// returned. Otherwise, the method tries to find the implementation by
  /// looking through the interface and its implementation. If a method
  /// implementation is found, it is returned otherwise `MD` is returned.
  const clang::ObjCMethodDecl* FindMethodDefn(
      const clang::ObjCMethodDecl* MD, const clang::ObjCInterfaceDecl* I);

  void VisitObjCInterfaceDeclComment(const clang::ObjCInterfaceDecl* Decl,
                                     const clang::RawComment* Comment,
                                     const clang::DeclContext* DCxt,
                                     std::optional<GraphObserver::NodeId> DCID);

  void VisitRecordDeclComment(const clang::RecordDecl* Decl,
                              const clang::RawComment* Comment,
                              const clang::DeclContext* DCxt,
                              std::optional<GraphObserver::NodeId> DCID);

  /// \brief Returns whether `Decl` should be indexed.
  bool ShouldIndex(const clang::Decl* Decl);

  /// \brief Binds a particular `target` with a semantic `kind` (e.g., this
  /// expr is a use that is a write to some node).
  struct KindWithTarget {
    GraphObserver::UseKind kind;   ///< The way `target` is being used.
    GraphObserver::NodeId target;  ///< The node being used.
  };

  KindWithTarget UseKindFor(const clang::Expr* expr,
                            const GraphObserver::NodeId& default_target) {
    auto i = use_kinds_.find(expr);
    return i == use_kinds_.end()
               ? KindWithTarget{GraphObserver::UseKind::kUnknown,
                                default_target}
               : KindWithTarget{i->second.kind, i->second.target
                                                    ? *i->second.target
                                                    : default_target};
  }

  /// \brief Marks that `expr` was used as a write target.
  /// \return `expr` as passed, as well as an optional indirect target.
  TargetExpr UsedAsWrite(const clang::Expr* expr) {
    if (expr != nullptr) {
      auto [head, alt] = SkipTrivialAliasing(expr);
      use_kinds_[head] = {GraphObserver::UseKind::kWrite, alt};
      return {expr, alt};
    }
    return {expr, std::nullopt};
  }

  /// \brief Marks that `expr` was used as a read+write target.
  /// \return `expr` as passed, as well as an optional indirect target.
  TargetExpr UsedAsReadWrite(const clang::Expr* expr) {
    if (expr != nullptr) {
      auto [head, alt] = SkipTrivialAliasing(expr);
      use_kinds_[head] = {GraphObserver::UseKind::kReadWrite, alt};
      return {expr, alt};
    }
    return {expr, std::nullopt};
  }

  /// \brief Returns a bundle of nodes to blame for field initializers.
  IndexJob::SomeNodes FindConstructorsForBlame(const clang::FieldDecl& field);
  IndexJob::SomeNodes FindNodesForBlame(const clang::ObjCMethodDecl& decl);
  IndexJob::SomeNodes FindNodesForBlame(const clang::FunctionDecl& decl);

  /// \brief Maps known Decls to their NodeIds.
  llvm::DenseMap<const clang::Decl*, GraphObserver::NodeId> DeclToNodeId;

  /// \brief Enabled library-specific callbacks.
  const LibrarySupports& Supports;

  /// \brief The `Sema` instance to use.
  clang::Sema& Sema;

  /// \brief The cache to use to generate signatures.
  MarkedSourceCache MarkedSources;

  /// \brief The active indexing job.
  std::unique_ptr<IndexJob> Job;

  /// \brief The controlling worklist.
  std::unique_ptr<IndexerWorklist> Worklist;

  /// \brief Comments we've already visited.
  std::unordered_set<const clang::RawComment*> VisitedComments;

  /// \brief Binds a particular implicit `expr` with a semantic `kind` (e.g.,
  /// this expr is a use that is a write to some node). May provide an
  /// overriding `target` if there is a more specific node than the one
  /// implied by the `expr`.
  struct KindWithOptionalTarget {
    GraphObserver::UseKind kind;  ///< The way `target` is being used.
    std::optional<GraphObserver::NodeId>
        target;  ///< The node being used, if different from
                 ///< the implicit `expr`.
  };

  /// \brief AST nodes we know are used in specific ways.
  absl::flat_hash_map<const clang::Expr*, KindWithOptionalTarget> use_kinds_;

  struct AlternateSemantic {
    GraphObserver::UseKind use_kind;
    std::optional<GraphObserver::NodeId> node;
  };

  /// \return the alternate semantic for `decl` or null.
  AlternateSemantic* AlternateSemanticForDecl(const clang::Decl* decl);

  /// \brief Maps from declaring token (begin, end) locations (as pairs of
  /// encoded clang::SourceLocations) to alternate semantics.
  absl::flat_hash_map<std::pair<unsigned, unsigned>, AlternateSemantic>
      alternate_semantic_cache_;

  /// \brief Decls known to have alternate semantics.
  absl::flat_hash_map<const clang::Decl*, AlternateSemantic*>
      alternate_semantics_;

  /// \brief Configurable indexer options.
  const IndexerOptions& options_;

  /// \brief Used for calculating semantic hashes.
  SemanticHash Hash;
};

/// \brief An `ASTConsumer` that passes events to a `GraphObserver`.
class IndexerASTConsumer : public clang::SemaConsumer {
 public:
  explicit IndexerASTConsumer(GraphObserver* GO, const LibrarySupports& S,
                              const IndexerOptions& IO
                                  ABSL_ATTRIBUTE_LIFETIME_BOUND)
      : Observer(GO), Supports(S), options_(IO) {}

  void HandleTranslationUnit(clang::ASTContext& Context) override {
    CHECK(Sema != nullptr);
    IndexerASTVisitor Visitor(Context, Supports, *Sema, options_, Observer);
    {
      ProfileBlock block(Observer->getProfilingCallback(), "traverse_tu");
      Visitor.PrepareAlternateSemanticCache();
      Visitor.Work(Context.getTranslationUnitDecl(),
                   options_.CreateWorklist(&Visitor));
    }
  }

  void InitializeSema(clang::Sema& S) override { Sema = &S; }

  void ForgetSema() override { Sema = nullptr; }

 private:
  GraphObserver* const Observer;
  /// Which library supports are enabled.
  const LibrarySupports& Supports;
  /// The active Sema instance.
  clang::Sema* Sema;
  /// \brief Configurable options for the indexer.
  const IndexerOptions& options_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_INDEXER_AST_HOOKS_H_
