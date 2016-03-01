/*
 * Copyright 2015 Google Inc. All rights reserved.
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

#include "clang/AST/Decl.h"

#include "kythe/cxx/common/indexing/KytheGraphRecorder.h"

#include "GraphObserver.h"

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
    const clang::Decl *Decl;
    /// The corresponding NodeId. If `Decl` is underneath a template, this will
    /// point to the `Abs` node of that template.
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
  /// \param DeclNodeId If `Decl` is under a template, the `NodeId` of the
  /// surrounding `Abs`; otherwise, the `NodeId` of the `Decl`.
  /// \param DeclBodyNodeId The variable's NodeId (not its surrounding Abs)
  /// \param Decl The VarDecl in question.
  /// \param Compl Whether the Decl was complete.
  /// \param Compls If the Decl is complete, the decls that it completes.
  virtual void InspectVariable(IndexerASTVisitor &V,
                               GraphObserver::NodeId &DeclNodeId,
                               GraphObserver::NodeId &DeclBodyNodeId,
                               const clang::VarDecl *Decl,
                               GraphObserver::Completeness Compl,
                               const std::vector<Completion> &Compls) {}

  /// \brief Called on any DeclRef.
  /// \param V The active IndexerASTVisitor.
  /// \param DeclRefLocation The location of the reference.
  /// \param Ref The range of the reference.
  /// \param RefId The NodeId of the referent (TargetDecl).
  /// \param TargetDecl The NamedDecl being referenced.
  virtual void InspectDeclRef(IndexerASTVisitor &V,
                              clang::SourceLocation DeclRefLocation,
                              const GraphObserver::Range &Ref,
                              GraphObserver::NodeId &RefId,
                              const clang::NamedDecl *TargetDecl) {}
};

/// \brief A collection of library support implementations.
using LibrarySupports = std::vector<std::unique_ptr<LibrarySupport>>;

/// \brief Snoops on variable declarations and references to see if they
/// are flags.
class GoogleFlagsLibrarySupport : public LibrarySupport {
public:
  GoogleFlagsLibrarySupport() {}

  /// \brief Emits a google/gflag node if `Decl` is a flag.
  void InspectVariable(IndexerASTVisitor &V, GraphObserver::NodeId &DeclNodeId,
                       GraphObserver::NodeId &DeclBodyNodeId,
                       const clang::VarDecl *Decl,
                       GraphObserver::Completeness Compl,
                       const std::vector<Completion> &Compls) override;

  /// \brief Checks whether this DeclRef refers to a flag. If so, it emits an
  /// additional ref edge to the correspondng google/gflag node.
  void InspectDeclRef(IndexerASTVisitor &V,
                      clang::SourceLocation DeclRefLocation,
                      const GraphObserver::Range &Ref,
                      GraphObserver::NodeId &RefId,
                      const clang::NamedDecl *TargetDecl) override;
};

} // namespace kythe

#endif // KYTHE_CXX_INDEXER_CXX_INDEXER_LIBRARY_SUPPORT_H_
