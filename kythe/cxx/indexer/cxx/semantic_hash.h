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
#ifndef KYTHE_CXX_INDEXER_CXX_SEMANTIC_HASH_H_
#define KYTHE_CXX_INDEXER_CXX_SEMANTIC_HASH_H_

#include <functional>

#include "clang/AST/Decl.h"
#include "clang/AST/DeclTemplate.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringExtras.h"

namespace kythe {

class SemanticHash {
 public:
  /// \brief Specifies what the hasher should do if it encounters a case it
  /// doesn't understand.
  enum class OnUnimplemented : bool {
    Abort = false,   ///< Stop indexing and exit with an error.
    Continue = true  ///< Continue indexing, possibly emitting less data.
  };

  /// \brief Constructs a new SemanticHash using `decl_string` to calculate a
  /// string from a template declaration and `on_unimplemented` for behavior
  /// when encountering an unhandled AST node.
  explicit SemanticHash(
      std::function<std::string(const clang::Decl*)> decl_string,
      OnUnimplemented on_unimplemented = OnUnimplemented::Abort)
      : ignore_unimplemented_(on_unimplemented == OnUnimplemented::Continue),
        decl_string_(std::move(decl_string)) {}

  // SemanticHash is copyable and more efficiently movable.
  SemanticHash(const SemanticHash&) = default;
  SemanticHash& operator=(const SemanticHash&) = default;
  SemanticHash(SemanticHash&&) = default;
  SemanticHash& operator=(SemanticHash&&) = default;

  /// \brief Builda semantic hash for the type, deferring to the
  /// appropriate overload of `Hash`.
  template <typename T>
  uint64_t operator()(T&& value) const {
    return this->Hash(std::forward<T>(value));
  }

  /// \brief Builds a semantic hash of the given `TemplateArgumentList`.
  uint64_t Hash(const clang::TemplateArgumentList* value) const;

  /// \brief Builds a semantic hash of the given `TemplateArgument`.
  uint64_t Hash(const clang::TemplateArgument& value) const;

  /// \brief Builds a semantic hash of the given `TemplateName`.
  uint64_t Hash(const clang::TemplateName& value) const;

  /// \brief Builds a semantic hash of the given `Type`, such that
  /// if T and T' are similar types, SH(T) == SH(T'). Note that the type is
  /// always canonicalized before its hash is taken.
  uint64_t Hash(const clang::QualType& value) const;

  /// \brief Builds a semantic hash of the given `RecordDecl`, such that
  /// if R and R' are similar records, SH(R) == SH(R'). This notion of
  /// similarity is meant to join together definitions copied and pasted
  /// across different translation units. As it is at best an approximation,
  /// it should be paired with the spelled-out name of the object being declared
  /// to form an identifying token.
  uint64_t Hash(const clang::RecordDecl* value) const;

  /// \brief Builds a semantic hash of the given `EnumDecl`, such that
  /// if E and E' are similar records, SH(E) == SH(E'). This notion of
  /// similarity is meant to join together definitions copied and pasted
  /// across different translation units. As it is at best an approximation,
  /// it should be paired with the spelled-out name of the object being declared
  /// to form an identifying token.
  uint64_t Hash(const clang::EnumDecl* value) const;

  uint64_t Hash(const clang::Selector& value) const;

 private:
  /// \brief Builds a semantic hash of the given `Decl`, which should look
  /// like a `TemplateDecl` (eg, a `TemplateDecl` itself or a partial
  /// specialization).
  uint64_t HashTemplateDeclish(const clang::Decl* decl) const;

  /// \brief Whether or not to ignore unimplemented nodes.
  bool ignore_unimplemented_;

  /// \brief Function to call when generating strings for template Decls.
  std::function<std::string(const clang::Decl*)> decl_string_;

  /// \brief Maps EnumDecls to semantic hashes.
  mutable llvm::DenseMap<const clang::EnumDecl*, uint64_t> enum_cache_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_SEMANTIC_HASH_H_
