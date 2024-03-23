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
#ifndef KYTHE_CXX_INDEXER_CXX_TYPE_MAP_H_
#define KYTHE_CXX_INDEXER_CXX_TYPE_MAP_H_

#include <cstdint>
#include <functional>

#include "absl/container/flat_hash_map.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Type.h"

namespace kythe {

// A simple hashable (using TypeKey::Hash), EqComparable class for
// constructing unique keys from types.
class TypeKey {
 public:
  explicit TypeKey(const clang::ASTContext& context,
                   const clang::QualType& qual_type, const clang::Type* type);

  // Custom hash function for the opaque contained value.
  struct Hash : std::hash<std::uintptr_t> {
    std::size_t operator()(const TypeKey& key) const {
      return hash::operator()(key.value_);
    }
  };

  // Trivial equality operator for the opaque contained value.
  friend bool operator==(const TypeKey& lhs, const TypeKey& rhs) {
    return lhs.value_ == rhs.value_;
  }

  // Trivial inequality operator for the opaque contained value.
  friend bool operator!=(const TypeKey& lhs, const TypeKey& rhs) {
    return !(lhs == rhs);
  }

 private:
  std::uintptr_t value_;
};

// An unordered map using the above as key.
template <typename T>
using TypeMap = absl::flat_hash_map<TypeKey, T, TypeKey::Hash>;

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_TYPE_MAP_H_
