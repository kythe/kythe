/*
 * Copyright 2017 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_INDEXER_CXX_PROTO_LIBRARY_SUPPORT_H_
#define KYTHE_CXX_INDEXER_CXX_PROTO_LIBRARY_SUPPORT_H_

#include "IndexerLibrarySupport.h"

namespace kythe {

/// \brief Indexes protobufs provided as string literals with ParseProtoHelper.
class GoogleProtoLibrarySupport : public LibrarySupport {
 public:
  GoogleProtoLibrarySupport() {}

  void InspectCallExpr(IndexerASTVisitor& V, const clang::CallExpr* CallExpr,
                       const GraphObserver::Range& Range,
                       const GraphObserver::NodeId& CalleeId) override;

 private:
  // Lazily initializes ParseProtoHelperDecl, and returns true if
  // ParseProtoHelper is available.
  bool CompilationUnitHasParseProtoHelperDecl(
      const clang::ASTContext& ASTContext, const clang::CallExpr& Expr);

  bool Initialized = false;
  const clang::Decl* ParseProtoHelperDecl = nullptr;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_PROTO_LIBRARY_SUPPORT_H_
