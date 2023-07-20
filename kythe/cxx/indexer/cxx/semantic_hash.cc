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
#include "kythe/cxx/indexer/cxx/semantic_hash.h"

#include <string>

#include "absl/log/check.h"
#include "absl/log/log.h"

namespace kythe {

uint64_t SemanticHash::HashTemplateDeclish(const clang::Decl* decl) const {
  return std::hash<std::string>()(decl_string_(decl));
}

uint64_t SemanticHash::Hash(const clang::TemplateName& name) const {
  using ::clang::TemplateName;
  switch (name.getKind()) {
    case TemplateName::Template:
      return HashTemplateDeclish(name.getAsTemplateDecl());
    case TemplateName::OverloadedTemplate:
      CHECK(ignore_unimplemented_) << "SemanticHash(OverloadedTemplate)";
      return 0;
    case TemplateName::AssumedTemplate:
      CHECK(ignore_unimplemented_) << "SemanticHash(AssumedTemplate)";
      return 0;
    case TemplateName::QualifiedTemplate:
      CHECK(ignore_unimplemented_) << "SemanticHash(QualifiedTemplate)";
      return 0;
    case TemplateName::DependentTemplate:
      CHECK(ignore_unimplemented_) << "SemanticHash(DependentTemplate)";
      return 0;
    case TemplateName::SubstTemplateTemplateParm:
      CHECK(ignore_unimplemented_) << "SemanticHash(SubstTemplateTemplateParm)";
      return 0;
    case TemplateName::SubstTemplateTemplateParmPack:
      CHECK(ignore_unimplemented_)
          << "SemanticHash(SubstTemplateTemplateParmPack)";
      return 0;
    case TemplateName::UsingTemplate:
      CHECK(ignore_unimplemented_) << "SemanticHash(UsingTemplate)";
      return 0;
  }
  CHECK(ignore_unimplemented_)
      << "Unexpected TemplateName Kind: " << name.getKind();
  return 0;
}

uint64_t SemanticHash::Hash(const clang::TemplateArgument& arg) const {
  using ::clang::TemplateArgument;
  switch (arg.getKind()) {
    case TemplateArgument::Null:
      return 0x1010101001010101LL;  // Arbitrary constant for H(Null).
    case TemplateArgument::Type:
      return Hash(arg.getAsType()) ^ 0x2020202002020202LL;
    case TemplateArgument::Declaration:
      return Hash(arg.getParamTypeForDecl()) ^
             std::hash<std::string>()(
                 arg.getAsDecl()->getQualifiedNameAsString());
    case TemplateArgument::NullPtr:
      return 0;
    case TemplateArgument::Integral: {
      auto value = arg.getAsIntegral();
      if (value.getSignificantBits() <= sizeof(uint64_t) * CHAR_BIT) {
        return static_cast<uint64_t>(value.isSigned() ? value.getSExtValue()
                                                      : value.getZExtValue());
      } else {
        return std::hash<std::string>()(llvm::toString(value, 10));
      }
    }
    case TemplateArgument::Template:
      return Hash(arg.getAsTemplate()) ^ 0x4040404004040404LL;
    case TemplateArgument::TemplateExpansion:
      CHECK(ignore_unimplemented_) << "SemanticHash(TemplateExpansion)";
      return 0;
    case TemplateArgument::Expression:
      CHECK(ignore_unimplemented_) << "SemanticHash(Expression)";
      return 0;
    case TemplateArgument::Pack: {
      uint64_t out = 0x8080808008080808LL;
      for (const auto& element : arg.pack_elements()) {
        out = Hash(element) ^ ((out << 1) | (out >> 63));
      }
      return out;
    }
  }
  CHECK(ignore_unimplemented_)
      << "Unexpected TemplateArgument Kind: " << arg.getKind();
  return 0;
}

uint64_t SemanticHash::Hash(const clang::QualType& type) const {
  return std::hash<std::string>()(type.getCanonicalType().getAsString());
}

uint64_t SemanticHash::Hash(const clang::EnumDecl* decl) const {
  // Memoize semantic hashes for enums, as they are also needed to create
  // names for enum constants.
  auto inserted = enum_cache_.insert({decl, 0});
  if (!inserted.second) return inserted.first->second;

  // TODO(zarko): Do we need a better hash function?
  uint64_t hash = 0;
  for (auto member : decl->enumerators()) {
    if (member->getDeclName().isIdentifier()) {
      hash ^= std::hash<std::string>()(std::string(member->getName()));
    }
  }
  inserted.first->second = hash;
  return hash;
}

uint64_t SemanticHash::Hash(const clang::TemplateArgumentList* arg_list) const {
  uint64_t hash = 0;
  for (const auto& arg : arg_list->asArray()) {
    hash ^= Hash(arg);
  }
  return hash;
}

uint64_t SemanticHash::Hash(const clang::RecordDecl* decl) const {
  // TODO(zarko): Do we need a better hash function? We may need to
  // hash the type variable context all the way up to the root template.
  uint64_t hash = 0;
  for (const auto* child : decl->decls()) {
    if (child->getDeclContext() != decl) {
      // Some decls appear underneath RD in the AST but aren't semantically
      // part of it. For example, in
      //   struct S { struct T *t; };
      // the RecordDecl for T is an AST child of S, but is a DeclContext
      // sibling.
      continue;
    }
    if (const auto* named_child = clang::dyn_cast<clang::NamedDecl>(child)) {
      if (named_child->getDeclName().isIdentifier()) {
        hash ^= std::hash<std::string>()(std::string(named_child->getName()));
      }
    }
  }
  if (const auto* record_decl = clang::dyn_cast<clang::CXXRecordDecl>(decl)) {
    if (const auto* tmpl_decl = record_decl->getDescribedClassTemplate()) {
      hash ^= HashTemplateDeclish(tmpl_decl);
    }
  }
  if (const auto* spec_decl =
          clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(decl)) {
    hash ^= Hash(clang::QualType(spec_decl->getTypeForDecl(), 0));
  }
  return hash;
}

uint64_t SemanticHash::Hash(const clang::Selector& selector) const {
  return std::hash<std::string>()(selector.getAsString());
}

}  // namespace kythe
