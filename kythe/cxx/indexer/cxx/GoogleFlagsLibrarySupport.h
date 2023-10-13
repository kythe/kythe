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

#ifndef KYTHE_CXX_INDEXER_CXX_GOOGLE_FLAGS_LIBRARY_SUPPORT_H_
#define KYTHE_CXX_INDEXER_CXX_GOOGLE_FLAGS_LIBRARY_SUPPORT_H_

#include "GraphObserver.h"
#include "IndexerLibrarySupport.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"

namespace kythe {

/// \brief Snoops on variable declarations and references to see if they
/// are flags.
class GoogleFlagsLibrarySupport : public LibrarySupport {
 public:
  GoogleFlagsLibrarySupport() {}

  /// \brief Emits a google/gflag node if `Decl` is a flag.
  void InspectVariable(IndexerASTVisitor& V,
                       const GraphObserver::NodeId& DeclNodeId,
                       const clang::VarDecl* Decl,
                       GraphObserver::Completeness Compl,
                       const std::vector<Completion>& Compls) override;

  /// \brief Checks whether this DeclRef refers to a flag. If so, it emits an
  /// additional ref edge to the correspondng google/gflag node.
  void InspectDeclRef(IndexerASTVisitor& V,
                      clang::SourceLocation DeclRefLocation,
                      const GraphObserver::Range& Ref,
                      const GraphObserver::NodeId& RefId,
                      const clang::NamedDecl* TargetDecl) override;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_GOOGLE_FLAGS_LIBRARY_SUPPORT_H_
