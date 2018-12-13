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
#ifndef KYTHE_CXX_INDEXER_CXX_RECURSIVE_TYPE_VISITOR_H_
#define KYTHE_CXX_INDEXER_CXX_RECURSIVE_TYPE_VISITOR_H_

#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"

namespace kythe {

// RecursiveTypeVisitor is a parallel type to clang::RecursiveASTVisitor which
// uses the same visitation strategy, but visits type-as-written and resolved
// type in parallel.
template <typename Derived>
class RecursiveTypeVisitor {
 public:
  Derived& getDerived() { return *static_cast<Derived*>(this); }
  /// Recursively vist a type-as-written with location in parallel
  /// with the derived type, by dispatching to Traverse*TypeLocType()
  /// based on the TypeLoc argument's getTypeClass() property.
  ///
  /// \returns false if the visitation was terminated early,
  /// true otherwise (including when the argument is a Null type location).
  bool TraverseTypeLocType(clang::TypeLoc TL, clang::QualType T);

  // Declare Traverse*() for all concrete TypeLoc classes.
  // Note: We're using TypeNodes.def as QualifiedTypeLoc needs to be handled
  // specially.
#define ABSTRACT_TYPE(CLASS, BASE)
#define TYPE(CLASS, BASE)                                     \
  bool Traverse##CLASS##TypeLocType(clang::CLASS##TypeLoc TL, \
                                    const clang::CLASS##Type* T);
#include "clang/AST/TypeNodes.def"

  bool TraverseQualifiedTypeLocType(clang::QualifiedTypeLoc TL,
                                    clang::QualType T);

  // Define WalkUpFrom*() and empty Visit*() for all TypeLoc classes.
  bool WalkUpFromTypeLocType(clang::TypeLoc TL, const clang::Type* T) {
    return getDerived().VisitTypeLocType(TL, T);
  }
  bool VisitTypeLocType(clang::TypeLoc TL, const clang::Type* T) {
    return true;
  }

  // QualifiedTypeLoc and UnqualTypeLoc are not declared in
  // TypeNodes.def and thus need to be handled specially.
  bool WalkUpFromQualifiedTypeLoc(clang::QualifiedTypeLoc TL) {
    return getDerived().VisitUnqualTypeLoc(TL.getUnqualifiedLoc());
  }
  bool VisitQualifiedTypeLoc(clang::QualifiedTypeLoc TL) { return true; }
  bool WalkUpFromUnqualTypeLoc(clang::UnqualTypeLoc TL) {
    return getDerived().VisitUnqualTypeLoc(TL.getUnqualifiedLoc());
  }
  bool VisitUnqualTypeLoc(clang::UnqualTypeLoc TL) { return true; }

  // Note that BASE includes trailing 'Type' which CLASS doesn't.
#define TYPE(CLASS, BASE)                                            \
  bool WalkUpFrom##CLASS##TypeLocType(clang::CLASS##TypeLoc TL,      \
                                      const clang::CLASS##Type* T) { \
    return getDerived().WalkUpFrom##BASE##LocType(TL, T) &&          \
           getDerived().Visit##CLASS##TypeLocType(TL, T);            \
  }                                                                  \
  bool Visit##CLASS##TypeLocType(clang::CLASS##TypeLoc TL,           \
                                 const clang::CLASS##Type* T) {      \
    return true;                                                     \
  }
#include "clang/AST/TypeNodes.def"

 private:
#define ABSTRACT_TYPE(CLASS, BASE)
#define TYPE(CLASS, BASE)                                                      \
  bool Traverse##CLASS##TypeLocTypeHelper(clang::CLASS##TypeLoc TL,            \
                                          const clang::CLASS##Type* T) {       \
    return getDerived().Traverse##CLASS##TypeLocType(TL,                       \
                                                     T ? T : TL.getTypePtr()); \
  }
#include "clang/AST/TypeNodes.def"
};

template <typename Derived>
bool RecursiveTypeVisitor<Derived>::TraverseTypeLocType(clang::TypeLoc TL,
                                                        clang::QualType T) {
  if (TL.isNull()) return true;
  if (T.isNull()) {
    T = TL.getType();
  }

  switch (TL.getTypeLocClass()) {
    case clang::TypeLoc::Qualified:
      return getDerived().TraverseQualifiedTypeLocType(
          TL.castAs<clang::QualifiedTypeLoc>(), T);
#define ABSTRACT_TYPE(CLASS, BASE)
#define TYPE(CLASS, BASE)                      \
  case clang::TypeLoc::CLASS:                  \
    return Traverse##CLASS##TypeLocTypeHelper( \
        TL.castAs<clang::CLASS##TypeLoc>(),    \
        clang::dyn_cast<clang::CLASS##Type>(T.getTypePtr()));
#include "clang/AST/TypeNodes.def"
  }

  return true;
}

template <typename Derived>
bool RecursiveTypeVisitor<Derived>::TraverseQualifiedTypeLocType(
    clang::QualifiedTypeLoc TL, clang::QualType T) {
  // See RecursiveASTVisitor<Derived>::TraverseQualifiedTypeLoc notes for why
  // we're doing this, but essentially we pretend to have never seen the locally
  // qualified version to avoid redundant visitation.
  return TraverseTypeLocType(TL.getUnqualifiedLoc(),
                             T.getLocalUnqualifiedType());
}

#define DEF_TRAVERSE_TYPELOC(TYPE, CODE)                          \
  template <typename Derived>                                     \
  bool RecursiveTypeVisitor<Derived>::Traverse##TYPE##LocType(    \
      clang::TYPE##Loc TL, const clang::TYPE* T) {                \
    return getDerived().WalkUpFrom##TYPE##LocType(TL, T) && [&] { \
      CODE;                                                       \
      return true;                                                \
    }();                                                          \
  }

DEF_TRAVERSE_TYPELOC(BuiltinType, {});
// TODO(shahms): Finish ComplexType.
DEF_TRAVERSE_TYPELOC(ComplexType, {});
DEF_TRAVERSE_TYPELOC(PointerType, {
  return getDerived().TraverseTypeLocType(TL.getPointeeLoc(),
                                          T->getPointeeType());
});
DEF_TRAVERSE_TYPELOC(BlockPointerType, {
  return getDerived().TraverseTypeLocType(TL.getPointeeLoc(),
                                          T->getPointeeType());
});
DEF_TRAVERSE_TYPELOC(LValueReferenceType, {
  return getDerived().TraverseTypeLocType(TL.getPointeeLoc(),
                                          T->getPointeeType());
});
DEF_TRAVERSE_TYPELOC(RValueReferenceType, {
  return getDerived().TraverseTypeLocType(TL.getPointeeLoc(),
                                          T->getPointeeType());
});
DEF_TRAVERSE_TYPELOC(MemberPointerType, {
  return getDerived().TraverseTypeLocType(TL.getPointeeLoc(),
                                          T->getPointeeType());
});
DEF_TRAVERSE_TYPELOC(AdjustedType, {
  return getDerived().TraverseTypeLocType(TL.getOriginalLoc(),
                                          T->getOriginalType());
});
DEF_TRAVERSE_TYPELOC(DecayedType, {
  return getDerived().TraverseTypeLocType(TL.getOriginalLoc(),
                                          T->getOriginalType());
});
DEF_TRAVERSE_TYPELOC(ConstantArrayType, {
  return getDerived().TraverseTypeLocType(TL.getElementLoc(),
                                          T->getElementType());
});
DEF_TRAVERSE_TYPELOC(IncompleteArrayType, {
  return getDerived().TraverseTypeLocType(TL.getElementLoc(),
                                          T->getElementType());
});
DEF_TRAVERSE_TYPELOC(VariableArrayType, {
  return getDerived().TraverseTypeLocType(TL.getElementLoc(),
                                          T->getElementType());
});
DEF_TRAVERSE_TYPELOC(DependentSizedArrayType, {
  return getDerived().TraverseTypeLocType(TL.getElementLoc(),
                                          T->getElementType());
});
DEF_TRAVERSE_TYPELOC(DependentAddressSpaceType, {});

#undef DEF_TRAVERSE_TYPELOC

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_RECURSIVE_TYPE_VISITOR_H_
