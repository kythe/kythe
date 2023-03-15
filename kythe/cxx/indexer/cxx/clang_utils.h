/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_INDEXER_CXX_CLANG_UTILS_H_
#define KYTHE_CXX_INDEXER_CXX_CLANG_UTILS_H_

#include "clang/AST/Decl.h"
#include "clang/AST/DeclarationName.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"
#include "kythe/cxx/indexer/cxx/indexed_parent_map.h"

namespace kythe {
/// \return true if `DN` is an Objective-C selector.
bool isObjCSelector(const clang::DeclarationName& DN);

/// \brief If `decl` is an implicit template instantiation or specialization,
/// returns the primary template or the partial specialization being
/// instantiated. Otherwise, returns `decl`.
const clang::Decl* FindSpecializedTemplate(const clang::Decl* decl);

/// \brief Finds the root member template starting at `decl` (which can be
/// any decl; if it's not a member template, returns `decl`).
/// \param use_mts If false, always return `decl`. (This parameter is temporary
/// and is used for the abs deprecation.)
const clang::Decl* DereferenceMemberTemplates(const clang::Decl* decl,
                                              bool use_mts);

/// \return true if a reference to `decl` should be given blame context.
bool ShouldHaveBlameContext(const clang::Decl* decl);

/// \return the `Expr` that is at the lvalue head position of `expr`, or
/// null otherwise. For example, in `foo[x].bar(y).z`, the member expression
/// for `z` is in the root position.
const clang::Expr* FindLValueHead(const clang::Expr* expr);

/// \return the `Decl` that is the target of influence by an lexpression with
/// head `head`, or null. For example, in `foo[x].bar(y).z`, the target of
/// influence is the member decl for `z`.
const clang::Decl* GetInfluencedDeclFromLValueHead(const clang::Expr* head);
}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_CLANG_UTILS_H_
