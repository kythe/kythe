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
#include "type_map.h"

namespace kythe {
namespace {
// Use the arithmetic sum of the pointer value of clang::Type and the numerical
// value of CVR qualifiers as the unique key for a QualType.
// The size of clang::Type is 24 (as of 2/22/2013), and the maximum of the
// qualifiers (i.e., the return value of clang::Qualifiers::getCVRQualifiers() )
// is clang::Qualifiers::CVRMask which is 7. Therefore, uniqueness is satisfied.
std::uintptr_t ComputeKeyFromQualType(const clang::ASTContext& context,
                                      const clang::QualType& qual_type,
                                      const clang::Type* type) {
  return qual_type.getLocalQualifiers().getCVRQualifiers() +
         reinterpret_cast<std::uintptr_t>(
             (clang::isa<clang::TemplateSpecializationType>(type) &&
              !type->isDependentType())
                 ? context.getCanonicalType(type)
                 : type);
}

}  // namespace

TypeKey::TypeKey(const clang::ASTContext& context,
                 const clang::QualType& qual_type, const clang::Type* type)
    : value_(ComputeKeyFromQualType(context, qual_type, type)) {}

}  // namespace kythe
