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

#include <algorithm>
#include <vector>

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"
#include "kythe/cxx/common/scope_guard.h"

namespace kythe {

// RecursiveTypeVisitor is a subclass of clang::RecursiveASTVisitor which
//  visits type-as-written and resolved type in parallel, via "TypePair"
//  member functions.
template <typename Derived>
class RecursiveTypeVisitor : public clang::RecursiveASTVisitor<Derived> {
  using Base = clang::RecursiveASTVisitor<Derived>;

 public:
  Derived& getDerived() { return *static_cast<Derived*>(this); }

  bool TraverseDecl(clang::Decl* Decl) {
    if (auto* DD = clang::dyn_cast<clang::DeclaratorDecl>(Decl)) {
      if (DD->getTypeSourceInfo()) {
        auto cleanup = PushScope(decl_stack_, DD);
        return Base::TraverseDecl(Decl);
      }
    }
    return Base::TraverseDecl(Decl);
  }

  bool TraverseObjCPropertyDecl(clang::ObjCPropertyDecl* Decl) {
    if (!getDerived().shouldTraversePostOrder()) {
      if (!getDerived().WalkUpFromObjCPropertyDecl(Decl)) {
        return false;
      }
    }
    if (auto* TSI = Decl->getTypeSourceInfo()) {
      return getDerived().TraverseTypePair(TSI->getTypeLoc(), Decl->getType());
    }
    return getDerived().TraverseType(Decl->getType());
  }

  bool TraverseTypeLoc(clang::TypeLoc TL) {
    if (!decl_stack_.empty() &&
        decl_stack_.back()->getTypeSourceInfo()->getTypeLoc() == TL) {
      return getDerived().TraverseTypePair(TL, decl_stack_.back()->getType());
    }
    return getDerived().TraverseTypePair(TL, TL.getType());
  }

  /// Recursively vist a type-as-written with location in parallel
  /// with the derived type, by dispatching to Traverse*TypePair()
  /// based on the TypeLoc argument's getTypeClass() property.
  ///
  /// \returns false if the visitation was terminated early,
  /// true otherwise (including when the argument is a Null type location).
  bool TraverseTypePair(clang::TypeLoc TL, clang::QualType T);

  // Declare Traverse*() for all concrete TypeLoc classes.
  // Note: We're using TypeNodes.def as QualifiedTypeLoc needs to be handled
  // specially.
#define ABSTRACT_TYPE(CLASS, BASE)
#define TYPE(CLASS, BASE)                                  \
  bool Traverse##CLASS##TypePair(clang::CLASS##TypeLoc TL, \
                                 const clang::CLASS##Type* T);
#include "clang/AST/TypeNodes.def"

  bool TraverseQualifiedTypePair(clang::QualifiedTypeLoc TL, clang::QualType T);

  // Define WalkUpFrom*() and empty Visit*() for all TypeLoc classes.
  bool WalkUpFromTypePair(clang::TypeLoc TL, const clang::Type* T) {
    return getDerived().VisitTypePair(TL, T);
  }
  bool VisitTypePair(clang::TypeLoc TL, const clang::Type* T) { return true; }

  // Note that BASE includes trailing 'Type' which CLASS doesn't.
#define TYPE(CLASS, BASE)                                         \
  bool WalkUpFrom##CLASS##TypePair(clang::CLASS##TypeLoc TL,      \
                                   const clang::CLASS##Type* T) { \
    return getDerived().WalkUpFrom##BASE##Pair(TL, T) &&          \
           getDerived().Visit##CLASS##TypePair(TL, T);            \
  }                                                               \
  bool Visit##CLASS##TypePair(clang::CLASS##TypeLoc TL,           \
                              const clang::CLASS##Type* T) {      \
    return true;                                                  \
  }
#include "clang/AST/TypeNodes.def"

 private:
#define ABSTRACT_TYPE(CLASS, BASE)
#define TYPE(CLASS, BASE)                                                   \
  bool Traverse##CLASS##TypePairHelper(clang::CLASS##TypeLoc TL,            \
                                       const clang::CLASS##Type* T) {       \
    return getDerived().Traverse##CLASS##TypePair(TL,                       \
                                                  T ? T : TL.getTypePtr()); \
  }
#include "clang/AST/TypeNodes.def"

  std::vector<clang::DeclaratorDecl*> decl_stack_;
};

template <typename Derived>
bool RecursiveTypeVisitor<Derived>::TraverseTypePair(clang::TypeLoc TL,
                                                     clang::QualType T) {
  if (TL.isNull()) return true;
  if (T.isNull()) {
    T = TL.getType();
  }

  switch (TL.getTypeLocClass()) {
    case clang::TypeLoc::Qualified:
      return getDerived().TraverseQualifiedTypePair(
          TL.castAs<clang::QualifiedTypeLoc>(), T);
#define ABSTRACT_TYPE(CLASS, BASE)
#define TYPE(CLASS, BASE)                   \
  case clang::TypeLoc::CLASS:               \
    return Traverse##CLASS##TypePairHelper( \
        TL.castAs<clang::CLASS##TypeLoc>(), \
        clang::dyn_cast<clang::CLASS##Type>(T.getTypePtr()));
#include "clang/AST/TypeNodes.def"
  }

  return true;
}

template <typename Derived>
bool RecursiveTypeVisitor<Derived>::TraverseQualifiedTypePair(
    clang::QualifiedTypeLoc TL, clang::QualType T) {
  // See RecursiveASTVisitor<Derived>::TraverseQualifiedTypeLoc notes for why
  // we're doing this, but essentially we pretend to have never seen the locally
  // qualified version to avoid redundant visitation.
  return TraverseTypePair(TL.getUnqualifiedLoc(), T.getLocalUnqualifiedType());
}

#define DEF_TRAVERSE_TYPEPAIR(TYPE, CODE)                      \
  template <typename Derived>                                  \
  bool RecursiveTypeVisitor<Derived>::Traverse##TYPE##Pair(    \
      clang::TYPE##Loc TL, const clang::TYPE* T) {             \
    return getDerived().WalkUpFrom##TYPE##Pair(TL, T) && [&] { \
      CODE;                                                    \
      return true;                                             \
    }();                                                       \
  }

DEF_TRAVERSE_TYPEPAIR(BuiltinType, {});
// TODO(shahms): Finish ComplexType.
DEF_TRAVERSE_TYPEPAIR(ComplexType, {});
DEF_TRAVERSE_TYPEPAIR(PointerType, {
  return getDerived().TraverseTypePair(TL.getPointeeLoc(), T->getPointeeType());
});
DEF_TRAVERSE_TYPEPAIR(BlockPointerType, {
  return getDerived().TraverseTypePair(TL.getPointeeLoc(), T->getPointeeType());
});
DEF_TRAVERSE_TYPEPAIR(LValueReferenceType, {
  return getDerived().TraverseTypePair(TL.getPointeeLoc(), T->getPointeeType());
});
DEF_TRAVERSE_TYPEPAIR(RValueReferenceType, {
  return getDerived().TraverseTypePair(TL.getPointeeLoc(), T->getPointeeType());
});
DEF_TRAVERSE_TYPEPAIR(MemberPointerType, {
  return getDerived().TraverseTypePair(TL.getClassTInfo()->getTypeLoc(),
                                       clang::QualType(T->getClass(), 0)) &&
         getDerived().TraverseTypePair(TL.getPointeeLoc(), T->getPointeeType());
});
DEF_TRAVERSE_TYPEPAIR(AdjustedType, {
  return getDerived().TraverseTypePair(TL.getOriginalLoc(),
                                       T->getOriginalType());
});
DEF_TRAVERSE_TYPEPAIR(DecayedType, {
  return getDerived().TraverseTypePair(TL.getOriginalLoc(),
                                       T->getOriginalType());
});
DEF_TRAVERSE_TYPEPAIR(ConstantArrayType, {
  return getDerived().TraverseTypePair(TL.getElementLoc(),
                                       T->getElementType()) &&
         getDerived().TraverseStmt(TL.getSizeExpr());
});
DEF_TRAVERSE_TYPEPAIR(IncompleteArrayType, {
  return getDerived().TraverseTypePair(TL.getElementLoc(),
                                       T->getElementType()) &&
         getDerived().TraverseStmt(TL.getSizeExpr());
});
DEF_TRAVERSE_TYPEPAIR(VariableArrayType, {
  return getDerived().TraverseTypePair(TL.getElementLoc(),
                                       T->getElementType()) &&
         getDerived().TraverseStmt(TL.getSizeExpr());
});
DEF_TRAVERSE_TYPEPAIR(DependentSizedArrayType, {
  return getDerived().TraverseTypePair(TL.getElementLoc(),
                                       T->getElementType()) &&
         getDerived().TraverseStmt(TL.getSizeExpr());
});
DEF_TRAVERSE_TYPEPAIR(DependentAddressSpaceType, {
  return getDerived().TraverseStmt(TL.getTypePtr()->getAddrSpaceExpr()) &&
         getDerived().TraverseTypePair(TL.getPointeeTypeLoc(),
                                       T->getPointeeType());
});
DEF_TRAVERSE_TYPEPAIR(DependentSizedExtVectorType, {
  if (auto* Expr = TL.getTypePtr()->getSizeExpr()) {
    if (!getDerived().TraverseStmt(Expr)) {
      return false;
    }
  }
  return getDerived().TraverseType(TL.getTypePtr()->getElementType());
});
DEF_TRAVERSE_TYPEPAIR(VectorType, {
  return getDerived().TraverseType(TL.getTypePtr()->getElementType());
});
DEF_TRAVERSE_TYPEPAIR(DependentVectorType, {
  if (auto* Expr = TL.getTypePtr()->getSizeExpr()) {
    if (!getDerived().TraverseStmt(Expr)) {
      return false;
    }
  }
  return getDerived().TraverseType(TL.getTypePtr()->getElementType());
});
DEF_TRAVERSE_TYPEPAIR(ExtVectorType, {
  return getDerived().TraverseType(TL.getTypePtr()->getElementType());
});
DEF_TRAVERSE_TYPEPAIR(FunctionNoProtoType, {
  return getDerived().TraverseTypePair(TL.getReturnLoc(), T->getReturnType());
});
DEF_TRAVERSE_TYPEPAIR(FunctionProtoType, {
  if (!getDerived().TraverseTypePair(TL.getReturnLoc(), T->getReturnType())) {
    return false;
  }
  T = TL.getTypePtr();  // Use TL for the remainder.
  for (unsigned I = 0, E = TL.getNumParams(); I != E; ++I) {
    if (auto P = TL.getParam(I)) {
      if (!getDerived().TraverseDecl(P)) {
        return false;
      }
    } else if (I < T->getNumParams()) {
      if (!getDerived().TraverseType(T->getParamType(I))) {
        return false;
      }
    }
  }
  for (const auto& E : T->exceptions()) {
    if (!getDerived().TraverseType(E)) {
      return false;
    }
  }
  if (auto* NE = T->getNoexceptExpr()) {
    return getDerived().TraverseStmt(NE);
  }
  return true;
});
DEF_TRAVERSE_TYPEPAIR(UnresolvedUsingType, {});
DEF_TRAVERSE_TYPEPAIR(TypedefType, {});
DEF_TRAVERSE_TYPEPAIR(TypeOfExprType, {
  return getDerived().TraverseStmt(TL.getUnderlyingExpr());
});
DEF_TRAVERSE_TYPEPAIR(TypeOfType, {
  return getDerived().TraverseTypePair(TL.getUnderlyingTInfo()->getTypeLoc(),
                                       T->getUnderlyingType());
});
DEF_TRAVERSE_TYPEPAIR(DecltypeType, {
  return getDerived().TraverseStmt(TL.getUnderlyingExpr());
});
DEF_TRAVERSE_TYPEPAIR(UnaryTransformType, {
  return getDerived().TraverseTypePair(TL.getUnderlyingTInfo()->getTypeLoc(),
                                       T->getUnderlyingType());
});
DEF_TRAVERSE_TYPEPAIR(AutoType, {
  return getDerived().TraverseType(TL.getTypePtr()->getDeducedType());
});
DEF_TRAVERSE_TYPEPAIR(DeducedTemplateSpecializationType, {
  return getDerived().TraverseTemplateName(
             TL.getTypePtr()->getTemplateName()) &&
         getDerived().TraverseType(TL.getTypePtr()->getDeducedType());
});
DEF_TRAVERSE_TYPEPAIR(RecordType, {});
DEF_TRAVERSE_TYPEPAIR(EnumType, {});
DEF_TRAVERSE_TYPEPAIR(TemplateTypeParmType, {});
DEF_TRAVERSE_TYPEPAIR(SubstTemplateTypeParmType, {
  return getDerived().TraverseType(TL.getTypePtr()->getReplacementType());
});
DEF_TRAVERSE_TYPEPAIR(SubstTemplateTypeParmPackType, {
  return getDerived().TraverseTemplateArgument(
      TL.getTypePtr()->getArgumentPack());
});
DEF_TRAVERSE_TYPEPAIR(TemplateSpecializationType, {
  if (!getDerived().TraverseTemplateName(TL.getTypePtr()->getTemplateName())) {
    return false;
  }
  for (unsigned I = 0, E = TL.getNumArgs(); I != E; ++I) {
    if (!getDerived().TraverseTemplateArgumentLoc(TL.getArgLoc(I))) {
      return false;
    }
  }
  return true;
});
DEF_TRAVERSE_TYPEPAIR(InjectedClassNameType, {});
DEF_TRAVERSE_TYPEPAIR(ParenType, {
  return getDerived().TraverseTypePair(TL.getInnerLoc(), T->getInnerType());
});
DEF_TRAVERSE_TYPEPAIR(AttributedType, {
  return getDerived().TraverseTypePair(TL.getModifiedLoc(),
                                       T->getModifiedType());
});
DEF_TRAVERSE_TYPEPAIR(ElaboratedType, {
  if (auto QL = TL.getQualifierLoc()) {
    if (!getDerived().TraverseNestedNameSpecifierLoc(QL)) {
      return false;
    }
  }
  return getDerived().TraverseTypePair(TL.getNamedTypeLoc(), T->getNamedType());
});
DEF_TRAVERSE_TYPEPAIR(DependentNameType, {
  return getDerived().TraverseNestedNameSpecifierLoc(TL.getQualifierLoc());
});
DEF_TRAVERSE_TYPEPAIR(DependentTemplateSpecializationType, {
  if (auto QL = TL.getQualifierLoc()) {
    if (!getDerived().TraverseNestedNameSpecifierLoc(QL)) {
      return false;
    }
  }
  for (unsigned I = 0, E = TL.getNumArgs(); I != E; ++I) {
    if (!getDerived().TraverseTemplateArgumentLoc(TL.getArgLoc(I))) {
      return false;
    }
  }
  return true;
});
DEF_TRAVERSE_TYPEPAIR(PackExpansionType, {
  return getDerived().TraverseTypePair(TL.getPatternLoc(), T->getPattern());
});
DEF_TRAVERSE_TYPEPAIR(ObjCTypeParamType, {});
DEF_TRAVERSE_TYPEPAIR(ObjCInterfaceType, {});
DEF_TRAVERSE_TYPEPAIR(ObjCObjectType, {
  if (TL.getTypePtr()->getBaseType().getTypePtr() != TL.getTypePtr()) {
    if (!getDerived().TraverseTypePair(TL.getBaseLoc(), T->getBaseType())) {
      return false;
    }
  }

  auto TypeArgs = T->getTypeArgs();
  for (unsigned I = 0,
                E = std::min<unsigned>(TL.getNumTypeArgs(), TypeArgs.size());
       I != E; ++I) {
    if (!getDerived().TraverseTypePair(TL.getTypeArgTInfo(I)->getTypeLoc(),
                                       TypeArgs[I])) {
      return false;
    }
  }
  return true;
});
DEF_TRAVERSE_TYPEPAIR(ObjCObjectPointerType, {
  return getDerived().TraverseTypePair(TL.getPointeeLoc(), T->getPointeeType());
});
DEF_TRAVERSE_TYPEPAIR(AtomicType, {
  return getDerived().TraverseTypePair(TL.getValueLoc(), T->getValueType());
});
DEF_TRAVERSE_TYPEPAIR(PipeType, {
  return getDerived().TraverseTypePair(TL.getValueLoc(), T->getElementType());
});

#undef DEF_TRAVERSE_TYPEPAIR

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_RECURSIVE_TYPE_VISITOR_H_
