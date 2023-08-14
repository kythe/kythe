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

#ifndef KYTHE_CXX_INDEXER_CXX_INDEXER_LIBRARY_SUPPORT_H_
#define KYTHE_CXX_INDEXER_CXX_INDEXER_LIBRARY_SUPPORT_H_

#include "GraphObserver.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"

namespace kythe {

class IndexerASTVisitor;

/// \brief A plugin for the IndexerASTVisitor for emitting library-specific
/// nodes.
///
/// IndexerASTVisitor is responsible for emitting graph information only about
/// language-level objects. LibrarySupport plugins can use additional knowledge
/// about particular libraries to emit further semantic information.
class LibrarySupport {
 public:
  virtual ~LibrarySupport() {}

  /// \brief A single completed declaration (in the Kythe `completes` sense).
  struct Completion {
    /// The Decl being completed.
    const clang::Decl* Decl;
    /// The corresponding NodeId.
    GraphObserver::NodeId DeclId;
  };

  /// \brief Called when a variable is defined or declared, including
  /// events due to template instantiations.
  ///
  /// Some libraries manifest their objects as variables with known
  /// properties. This hook allows the indexer to work backward from those
  /// variables to generate graph relationships that reflect library-specific
  /// meaning.
  ///
  /// \param V The active IndexerASTVisitor.
  /// \param DeclNodeId The `NodeId` of the `Decl`.
  /// \param Decl The VarDecl in question.
  /// \param Compl Whether the Decl was complete.
  /// \param Compls If the Decl is complete, the decls that it completes.
  virtual void InspectVariable(IndexerASTVisitor& V,
                               const GraphObserver::NodeId& DeclNodeId,
                               const clang::VarDecl* Decl,
                               GraphObserver::Completeness Compl,
                               const std::vector<Completion>& Compls) {}

  /// \brief Called on any DeclRef.
  /// \param V The active IndexerASTVisitor.
  /// \param DeclRefLocation The location of the reference.
  /// \param Ref The range of the reference.
  /// \param RefId The NodeId of the referent (TargetDecl).
  /// \param TargetDecl The NamedDecl being referenced.
  virtual void InspectDeclRef(IndexerASTVisitor& V,
                              clang::SourceLocation DeclRefLocation,
                              const GraphObserver::Range& Ref,
                              const GraphObserver::NodeId& RefId,
                              const clang::NamedDecl* TargetDecl) {}

  /// \brief Called on any DeclRef. An overload of the above function that
  /// provides access to the expression.
  ///
  /// \param V The active IndexerASTVisitor.
  /// \param DeclRefLocation The location of the reference.
  /// \param Ref The range of the reference.
  /// \param RefId The NodeId of the referent (TargetDecl).
  /// \param TargetDecl The NamedDecl being referenced.
  /// \param Expr The Expr that references the NamedDecl
  virtual void InspectDeclRef(IndexerASTVisitor& V,
                              clang::SourceLocation DeclRefLocation,
                              const GraphObserver::Range& Ref,
                              const GraphObserver::NodeId& RefId,
                              const clang::NamedDecl* TargetDecl,
                              const clang::Expr* Expr) {
    InspectDeclRef(V, DeclRefLocation, Ref, RefId, TargetDecl);
  }

  /// \brief Called on any CallExpr.
  /// \param V The active IndexerASTVisitor.
  /// \param CallExpr The call expr.
  /// \param Range The range of the call expr.
  /// \param CalleeId The NodeId of the callee.
  virtual void InspectCallExpr(IndexerASTVisitor& V,
                               const clang::CallExpr* CallExpr,
                               const GraphObserver::Range& Range,
                               const GraphObserver::NodeId& CalleeId) {}

  /// \brief Called on any FunctionDecl.
  /// \param V The active IndexerASTVisitor.
  /// \param DeclNodeId The `NodeId` of the `Decl`.
  /// \param FunctionDecl The call expr.
  /// \param Compl Whether the Decl was complete.
  /// \param Compls If the Decl is complete, the decls that it completes.
  virtual void InspectFunctionDecl(IndexerASTVisitor& V,
                                   const GraphObserver::NodeId& DeclNodeId,
                                   const clang::FunctionDecl* FunctionDecl,
                                   GraphObserver::Completeness Compl,
                                   const std::vector<Completion>& Compls) {}
};

/// \brief A collection of library support implementations.
using LibrarySupports = std::vector<std::unique_ptr<LibrarySupport>>;

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_INDEXER_LIBRARY_SUPPORT_H_
