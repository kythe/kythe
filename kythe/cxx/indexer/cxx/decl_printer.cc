/*
 * Copyright 2023 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/indexer/cxx/decl_printer.h"

#include <cstddef>
#include <optional>
#include <string>

#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclarationName.h"
#include "clang/Basic/LangOptions.h"
#include "kythe/cxx/indexer/cxx/GraphObserver.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

namespace kythe {
namespace {

int FindParameterParentIndex(const clang::FunctionDecl& parent,
                             const clang::NamedDecl& decl) {
  int index = 0;
  for (const auto* param : parent.parameters()) {
    if (&decl == param) {
      return index;
    }
    ++index;
  }
  return -1;
}

const clang::LangOptions& GetDefaultLangOptions() {
  static const clang::LangOptions kDefaultLangOptions;
  return kDefaultLangOptions;
}

const clang::LangOptions& GetLangOptions(const GraphObserver& observer) {
  // Do something reasonable if no LangOptions have been specified.
  const clang::LangOptions* options = observer.getLangOptions();
  return options ? *options : GetDefaultLangOptions();
}
}  // namespace

std::string DeclPrinter::QualifiedId(const clang::Decl& decl) const {
  std::string result;
  llvm::raw_string_ostream out(result);
  PrintQualifiedId(decl, out);
  return result;
}

void DeclPrinter::PrintQualifiedId(const clang::Decl& decl,
                                   llvm::raw_ostream& out) const {
  PrintQualifiedId(RootTraversal(&parent_map_->get(), &decl), out);
}

void DeclPrinter::PrintQualifiedId(const RootTraversal& path,
                                   llvm::raw_ostream& out) const {
  bool missing_separator = false;
  for (const auto& context : path) {
    if (llvm::isa_and_present<clang::LinkageSpecDecl>(context.decl)) {
      // Ignore linkage specification blocks to support C headers that wrap
      // extern "C" in #ifdef __cplusplus.
      continue;
    } else if (missing_separator &&
               llvm::isa_and_present<clang::TemplateDecl>(context.decl)) {
      // We would rather name 'template <etc> class C' as C, not C:C, but
      // we also want to be able to give useful names to templates when they're
      // explicitly requested. Therefore:
      continue;
    }

    // TODO(zarko): Do we need to deal with nodes with no memoization data?
    // According to ASTTypeTrates.h:205, only Stmt, Decl, Type and
    // NestedNameSpecifier return memoization data. Can we claim an invariant
    // that if we start at any Decl, we will always encounter nodes with
    // memoization data?
    const std::optional<size_t> index = context.parent_index();
    if (!index.has_value()) {
      if (const auto* named =
              llvm::dyn_cast_or_null<clang::NamedDecl>(context.decl);
          // Make sure that we don't miss out on implicit nodes.
          named != nullptr && named->isImplicit()) {
        if (!PrintNamedDecl(*named, out)) {
          // Heroically try to come up with a disambiguating identifier,
          // even when the IndexedParentVector is empty. This can happen
          // in anonymous parameter declarations that belong to function
          // prototypes.
          const auto* parent =
              llvm::dyn_cast<clang::FunctionDecl>(named->getDeclContext());
          if (parent != nullptr) {
            out << FindParameterParentIndex(*parent, *named) << ":";
            // Resume printing parent context from our DeclContext rather than
            // traversal parent.
            // TODO(shahms): This should be done as part of traversal, not as a
            // hack here; this is just another source for the index and parent.
            PrintQualifiedId(*parent, out);
            break;
          }
          out << "@unknown@";
        }
      }
      break;
    }
    if (missing_separator) {
      out << ":";
    } else {
      missing_separator = true;
    }

    // TODO(zarko): check for other specializations and emit accordingly
    // Alternately, maybe it would be better to just always emit the hash?
    // At any rate, a hash cache might be a good idea.
    if (!PrintName(context.decl, out)) {
      // If there's no good name for this Decl, name it after its child
      // index wrt its parent node.
      out << *index;
    }
  }
}

bool DeclPrinter::PrintName(const clang::Decl* decl,
                            llvm::raw_ostream& out) const {
  if (const auto* named = llvm::dyn_cast_or_null<clang::NamedDecl>(decl)) {
    return PrintNamedDecl(*named, out);
  }
  return false;
}

bool DeclPrinter::PrintNamedDecl(const clang::NamedDecl& decl,
                                 llvm::raw_ostream& out) const {
  if (PrintDeclarationName(decl.getDeclName(), out)) {
    return true;
  }

  // NamedDecls with empty names, e.g. unnamed namespaces, enums, structs, etc.
  if (llvm::isa<clang::NamespaceDecl>(decl)) {
    // This is an anonymous namespace. We have two cases:
    // If there is any file that is not a textual include (.inc,
    //     or explicitly marked as such in a module) between the declaration
    //     site and the main source file, then the namespace's identifier is
    //     the shared anonymous namespace identifier "@#anon"
    // Otherwise, it's the anonymous namespace identifier associated with the
    //     main source file.
    if (observer().isMainSourceFileRelatedLocation(decl.getLocation())) {
      observer().AppendMainSourceFileIdentifierToStream(out);
    }
    out << "@#anon";
    return true;
  } else if (const auto* recdecl = llvm::dyn_cast<clang::RecordDecl>(&decl)) {
    out << HashToString(hasher().Hash(recdecl));
    return true;
  } else if (const auto* enumdecl = llvm::dyn_cast<clang::EnumDecl>(&decl)) {
    out << HashToString(hasher().Hash(enumdecl));
    return true;
  }
  return false;
}

bool DeclPrinter::PrintDeclarationName(const clang::DeclarationName& name,
                                       llvm::raw_ostream& out) const {
  using ::clang::DeclarationName;
  switch (name.getNameKind()) {
    // We can reasonably rely on the default behavior for these.
    case DeclarationName::ObjCZeroArgSelector:
    case DeclarationName::ObjCOneArgSelector:
    case DeclarationName::ObjCMultiArgSelector:
    case DeclarationName::CXXOperatorName:
    case DeclarationName::CXXLiteralOperatorName:
    case DeclarationName::CXXConversionFunctionName:
      name.print(out, GetLangOptions(observer()));
      return true;

    // We rely on checking the class name for an empy identifier for
    // constructors and destructors.
    case DeclarationName::CXXDestructorName:
      out << '~';
      [[fallthrough]];
    case DeclarationName::CXXConstructorName:
      if (const auto type = name.getCXXNameType(); !type.isNull()) {
        // UnresolvedUsingValueDecl (and potentially others) result in a
        // non-null QualType, but a null CXXRecordDecl.
        return PrintName(type->getAsCXXRecordDecl(), out);
      }
      break;
    // These will be given parent-relative identifiers.
    case DeclarationName::CXXDeductionGuideName:
    case DeclarationName::CXXUsingDirective:
      return false;

    // Identifiers are handled specially to check for empty names,
    // which happens outside the switch statement.
    case DeclarationName::Identifier:
      if (const auto* ident = name.getAsIdentifierInfo();
          ident != nullptr && !ident->getName().empty()) {
        name.print(out, GetLangOptions(observer()));
        return true;
      }
      break;
  }
  return false;
}
}  // namespace kythe
