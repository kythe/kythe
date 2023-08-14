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

#ifndef KYTHE_CXX_INDEXER_CXX_IMPUTED_CONSTRUCTOR_SUPPORT_H_
#define KYTHE_CXX_INDEXER_CXX_IMPUTED_CONSTRUCTOR_SUPPORT_H_

#include <functional>
#include <string>
#include <unordered_set>

#include "IndexerLibrarySupport.h"
#include "absl/strings/string_view.h"
#include "clang/AST/Expr.h"

namespace kythe {

/// \brief Emits additional direct references to constructors when invoking
/// forwarding factor functions like `make_unique`.
class ImputedConstructorSupport : public LibrarySupport {
 public:
  explicit ImputedConstructorSupport(
      std::unordered_set<std::string> allowed_constructor_patterns = {
          "std(::\\w+)*::make_(unique|shared)", "absl::make_unique",
          "llvm::make_unique"});

  explicit ImputedConstructorSupport(
      std::function<bool(absl::string_view)> allow_constructor_name);

  void InspectCallExpr(IndexerASTVisitor& visitor,
                       const clang::CallExpr* call_expr,
                       const GraphObserver::Range& range,
                       const GraphObserver::NodeId& callee_id) override;

 private:
  const std::function<bool(absl::string_view)> allow_constructor_name_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_IMPUTED_CONSTRUCTOR_SUPPORT_H_
