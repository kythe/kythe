/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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
#include "indexed_parent_iterator.h"

namespace kythe {
namespace {
const clang::Decl* GetDeclInContext(const clang::Decl* decl) {
  if (decl == nullptr) return nullptr;
  if (const auto* tag_decl = clang::dyn_cast<clang::TagDecl>(decl)) {
    // Names for declarator-embedded decls should reflect lexical scope, not AST
    // scope.
    if (tag_decl->isEmbeddedInDeclarator()) {
      return clang::dyn_cast<clang::Decl>(tag_decl->getLexicalDeclContext());
    }
  }
  return nullptr;
}
}  // namespace

RootTraversal::iterator& RootTraversal::iterator::operator++() {
  advance();
  return *this;
}

RootTraversal::iterator RootTraversal::iterator::operator++(int) {
  iterator current = *this;
  advance();
  return current;
}

RootTraversal::iterator RootTraversal::iterator::next() const {
  if (current_->indexed_parent) {
    if (const auto* decl = GetDeclInContext(current_->decl)) {
      return iterator(parent_map_, decl);
    }
    return iterator(parent_map_, current_->indexed_parent->parent);
  }
  return iterator();
}

void RootTraversal::iterator::advance() { *this = next(); }

}  // namespace kythe
