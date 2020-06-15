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

#include "kythe/cxx/indexer/cxx/clang_utils.h"

#include "clang/AST/DeclTemplate.h"
#include "clang/Basic/CharInfo.h"
#include "clang/Lex/Lexer.h"
#include "glog/logging.h"

namespace kythe {
bool isObjCSelector(const clang::DeclarationName& DN) {
  switch (DN.getNameKind()) {
    case clang::DeclarationName::NameKind::ObjCZeroArgSelector:
    case clang::DeclarationName::NameKind::ObjCOneArgSelector:
    case clang::DeclarationName::NameKind::ObjCMultiArgSelector:
      return true;
    default:
      return false;
  }
}

clang::SourceLocation GetLocForEndOfToken(
    const clang::SourceManager& source_manager,
    const clang::LangOptions& lang_options,
    clang::SourceLocation start_location) {
  if (start_location.isMacroID()) {
    start_location = source_manager.getExpansionLoc(start_location);
  }
  return clang::Lexer::getLocForEndOfToken(start_location,
                                           0 /* offset from end of token */,
                                           source_manager, lang_options);
}

const clang::Decl* FindSpecializedTemplate(const clang::Decl* decl) {
  if (const auto* FD = llvm::dyn_cast<const clang::FunctionDecl>(decl)) {
    if (auto* ftsi = FD->getTemplateSpecializationInfo()) {
      if (!ftsi->isExplicitInstantiationOrSpecialization()) {
        return ftsi->getTemplate();
      }
    }
  } else if (const auto* ctsd =
                 llvm::dyn_cast<const clang::ClassTemplateSpecializationDecl>(
                     decl)) {
    if (!ctsd->isExplicitInstantiationOrSpecialization()) {
      auto primary_or_partial = ctsd->getSpecializedTemplateOrPartial();
      if (const auto* partial =
              primary_or_partial
                  .dyn_cast<clang::ClassTemplatePartialSpecializationDecl*>()) {
        return partial;
      } else if (const auto* primary =
                     primary_or_partial.dyn_cast<clang::ClassTemplateDecl*>()) {
        return primary;
      }
    }
  } else if (const auto* vtsd =
                 llvm::dyn_cast<const clang::VarTemplateSpecializationDecl>(
                     decl)) {
    if (!vtsd->isExplicitInstantiationOrSpecialization()) {
      auto primary_or_partial = vtsd->getSpecializedTemplateOrPartial();
      if (const auto* partial =
              primary_or_partial
                  .dyn_cast<clang::VarTemplatePartialSpecializationDecl*>()) {
        return partial;
      } else if (const auto* primary =
                     primary_or_partial.dyn_cast<clang::VarTemplateDecl*>()) {
        return primary;
      }
    }
  }
  return decl;
}

bool ShouldHaveBlameContext(const clang::Decl* decl) {
  // TODO(zarko): introduce more blameable decls.
  switch (decl->getKind()) {
    case clang::Decl::Kind::Var:
      return true;
    default:
      return false;
  }
}
}  // namespace kythe
