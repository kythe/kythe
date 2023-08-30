//- @pkg ref Package
//- Package code PackageCode
//- PackageCode child.1 PackageId
//- PackageId.kind "IDENTIFIER"
//- PackageId.pre_text "pkg"
package pkg;

import java.util.List;

@SuppressWarnings("unused")
//- @MarkedSource defines/binding Class
//- Class childof Package
//- Class code ClassId
//- ClassId child.1 ClassCxt
//- ClassCxt child.0 ClassCxtPackage
//- ClassCxtPackage.pre_text "pkg"
//- ClassId child.2 ClassTok
//- ClassTok.kind "IDENTIFIER"
//- ClassTok.pre_text "MarkedSource"
public class MarkedSource {

  // Implicit static class initializer
  //- ClassInit.node/kind function
  //- ClassInit childof Class
  //- ClassInit code InitCode
  //- InitCode child.0 InitContext
  //- InitCode.post_child_text "."
  //- InitContext.kind "CONTEXT"
  //- InitContext.pre_text "pkg.MarkedSource"
  //- InitCode child.1 InitIdent
  //- InitIdent.kind "IDENTIFIER"
  //- InitIdent.pre_text "<clinit>"

  //- @CONSTANT defines/binding Constant
  //- Constant code CMS
  //- CMS child.1 ConstantType
  //- ConstantType.kind "TYPE"
  //- CMS child.2 ConstantCtx
  //- ConstantCtx.kind "CONTEXT"
  //- CMS child.3 ConstantIdent
  //- ConstantIdent.kind "IDENTIFIER"
  //- CMS child.4 ConstantInit
  //- ConstantInit.kind "INITIALIZER"
  //- ConstantInit.pre_text "\"value\""
  public static final String CONSTANT = "value";

  //- @"fieldName$1" defines/binding Field
  //- Field childof Class
  //- @int ref Int
  //- Field code FieldTypeId
  //- FieldTypeId child.0 FieldType
  //- FieldTypeId child.1 FieldCxt
  //- FieldTypeId child.2 FieldId
  //- FieldType.kind "TYPE"
  //- FieldType.post_text " "
  //- FieldType child.0 FieldTypeIdent
  //- FieldTypeIdent.pre_text "int"
  //- FieldCxt child.1 FieldCxtClass
  //- FieldCxtClass.pre_text "MarkedSource"
  //- FieldId.kind "IDENTIFIER"
  //- FieldId.pre_text "fieldName$1"
  int fieldName$1;

  //- @MarkedSource defines/binding Ctor
  //- Ctor childof Class
  //- Ctor typed CType
  //- CType param.1 Class
  //- Ctor code CtorTypeCxtId
  //- CtorTypeCxtId child.1 CtorCxt
  //- CtorTypeCxtId child.2 CtorTok
  //- CtorTypeCxtId child.3 CtorParams
  //- CtorCxt child.1 CtorCxtClass
  //- CtorCxtClass.pre_text "MarkedSource"
  //- CtorTok.pre_text "MarkedSource"
  //- CtorParams.kind "PARAMETER_LOOKUP_BY_PARAM"
  //- CtorParams.pre_text "("
  //- CtorParams.post_child_text ", "
  //- CtorParams.post_text ")"
  public MarkedSource() {}

  //- @methodName defines/binding Method
  //- Method childof Class
  //- Method typed MType
  //- MType param.1 Void
  //- Method code MethodTypeCxtId
  //- MethodTypeCxtId child.0 MethodType
  //- MethodTypeCxtId child.1 MethodCxt
  //- MethodTypeCxtId child.2 MethodTok
  //- MethodTypeCxtId child.3 MethodParams
  //- MethodType.post_text " "
  //- MethodType child.0 MethodVoid
  //- MethodVoid.pre_text "void"
  //- MethodCxt child.1 MethodCxtClass
  //- MethodCxtClass.pre_text "MarkedSource"
  //- MethodTok.pre_text "methodName"
  //- MethodParams.kind "PARAMETER_LOOKUP_BY_PARAM"
  //- MethodParams.pre_text "("
  //- MethodParams.post_child_text ", "
  //- MethodParams.post_text ")"
  void methodName() {}

  //- @methodWithParams defines/binding MethodWithParams
  //- MethodWithParams typed MPType
  //- MPType param.1 Void
  //- MPType param.3 String
  //- MPType param.4 Int
  //- MPType code MethodTypeCode
  void methodWithParams(String a, int b) {}

  //- FnTypeCode.kind "TYPE"
  //- FnTypeCode child.0 FnTypeRetCode
  //- FnTypeRetCode.kind "LOOKUP_BY_PARAM"
  //- FnTypeRetCode.lookup_index 1
  //- FnTypeCode child.1 FnTypeParamsCode
  //- FnTypeParamsCode.kind "PARAMETER_LOOKUP_BY_PARAM"
  //- FnTypeParamsCode.pre_text "("
  //- FnTypeParamsCode.post_text ")"
  //- FnTypeParamsCode.post_child_text ", "
  //- FnTypeParamsCode.lookup_index 2

  //- MethodTypeCode.kind "TYPE"
  //- MethodTypeCode child.0 MethodTypeRetBox
  //- MethodTypeRetBox.kind "BOX"
  //- MethodTypeRetBox.post_text " "
  //- MethodTypeRetBox child.0 MethodTypeRetCode
  //- MethodTypeRetCode.kind "LOOKUP_BY_PARAM"
  //- MethodTypeRetCode.lookup_index 1
  //- MethodTypeCode child.1 MethodTypeRecvBox
  //- MethodTypeRecvBox.kind "BOX"
  //- MethodTypeRecvBox.post_text "::"
  //- MethodTypeRecvBox child.0 MethodTypeRecvCode
  //- MethodTypeRecvCode.kind "LOOKUP_BY_PARAM"
  //- MethodTypeRecvCode.lookup_index 2
  //- MethodTypeCode child.2 MethodTypeParamsCode
  //- MethodTypeParamsCode.kind "PARAMETER_LOOKUP_BY_PARAM"
  //- MethodTypeParamsCode.pre_text "("
  //- MethodTypeParamsCode.post_text ")"
  //- MethodTypeParamsCode.post_child_text ", "
  //- MethodTypeParamsCode.lookup_index 3

  //- @pa defines/binding AParam
  //- @pb defines/binding BParam
  //- @pc defines/binding CParam
  //- @pd defines/binding DParam
  void m(String pa, int pb, int[] pc, Object[] pd) {}

  //- AParam code AParamCode
  //- AParamCode child.0 APType
  //- APType.kind "TYPE"
  //- APType child.0 APTypeBox
  //- APTypeBox child.0 APTypeCtx
  //- APTypeCtx.kind "CONTEXT"
  //- APTypeBox child.1 APTypeId
  //- APTypeId.kind "IDENTIFIER"
  //- APTypeId.pre_text "String"
  //- APType.post_text " "
  //- AParamCode child.1 APCtx
  //- APCtx.kind "CONTEXT"
  //- AParamCode child.2 APId
  //- APId.kind "IDENTIFIER"
  //- APId.pre_text "pa"

  //- BParam code BParamCode
  //- BParamCode child.0 BPType
  //- BPType.kind "TYPE"
  //- BPType child.0 BPTypeId
  //- BPTypeId.kind "IDENTIFIER"
  //- BPTypeId.pre_text "int"
  //- BPType.post_text " "
  //- BParamCode child.1 BPCtx
  //- BPCtx.kind "CONTEXT"
  //- BParamCode child.2 BPId
  //- BPId.kind "IDENTIFIER"
  //- BPId.pre_text "pb"

  //- CParam code CParamCode
  //- CParamCode child.0 CPType
  //- CPType.kind "TYPE"
  //- CPType child.0 CPTypeId
  //- CPTypeId.kind "IDENTIFIER"
  //- CPTypeId.pre_text "int"
  //- CPTypeId.post_text "[]"
  //- CPType.post_text " "
  //- CParamCode child.1 CPCtx
  //- CPCtx.kind "CONTEXT"
  //- _CParamCPode child.2 CPId
  //- CPId.kind "IDENTIFIER"
  //- CPId.pre_text "pc"

  //- DParam code DParamCode
  //- DParamCode child.0 DPType
  //- DPType.kind "TYPE"
  //- DPType child.0 DPTypeBoxArray
  //- DPTypeBoxArray.post_text "[]"
  //- DPTypeBoxArray child.0 DPTypeBoxId
  //- DPTypeBoxId child.1 DPTypeId
  //- DPTypeId.kind "IDENTIFIER"
  //- DPTypeId.pre_text "Object"
  //- DPType.post_text " "
  //- DParamCode child.1 DPCtx
  //- DPCtx.kind "CONTEXT"
  //- _DParamDPode child.2 DPId
  //- DPId.kind "IDENTIFIER"
  //- DPId.pre_text "pd"

  //- @lst defines/binding ListArg
  //- ListArg code ListBox
  //- ListBox child.0 ListType
  //- ListType.kind "TYPE"
  //- ListType.post_text " "
  //- ListType child.0 ListBoxId
  //- ListBoxId child.1 ListId
  //- ListId.kind "IDENTIFIER"
  //- ListId.pre_text "List"
  //- ListType child.1 ListArgs
  //- ListArgs.kind "PARAMETER"
  //- ListArgs.pre_text "<"
  //- ListArgs.post_child_text ", "
  //- ListArgs.post_text ">"
  //- ListArgs child.0 ObjBoxId
  //- ObjBoxId child.0 ObjBox
  //- ObjBox child.0 ObjCtx
  //- ObjCtx.kind "CONTEXT"
  //- ObjBox child.1 ObjId
  //- ObjId.kind "IDENTIFIER"
  //- ObjId.pre_text "Object"
  void methodWithGeneric(List<Object> lst) {}

  // Ensure documentation is emitted for nodes referenced before their definitions.
  //- @Inner ref InnerClass
  static Inner refClassBeforeDef() {
    return null;
  }

  //- @Inner defines/binding InnerClass
  //- InnerClass code _
  public static class Inner {
    //- @field defines/binding IField
    //- IField code CIField
    //- CIField child.1 CIFieldCxt
    //- CIFieldCxt child.0 CIFieldPkg
    //- CIFieldPkg.pre_text "pkg"
    //- CIFieldCxt child.1 CIFieldMS
    //- CIFieldMS.pre_text "MarkedSource"
    //- CIFieldCxt child.2 CIFieldInner
    //- CIFieldInner.pre_text "Inner"
    int field;
  }

  static class InnerStatic<T extends Object> {
    //- @wildcardList defines/binding WildcardList
    //- WildcardList code WildBox
    //- WildBox child.0 WildListType
    //- WildListType.kind "TYPE"
    //- WildListType child.1 WildListTypeArgs
    //- WildListTypeArgs.kind "PARAMETER"
    //- WildListTypeArgs child.0 WildcardTypeArg
    //- WildcardTypeArg.pre_text "?"
    List<?> wildcardList;

    //- @genericList defines/binding GenericList
    //- GenericList code TBox
    //- TBox child.0 TListType
    //- TListType.kind "TYPE"
    //- TListType child.1 TListTypeArgs
    //- TListTypeArgs.kind "PARAMETER"
    //- TListTypeArgs child.0 GenericTypeArg
    //- GenericTypeArg.pre_text "T"
    List<T> genericList;

    //- @extendsBoundedList defines/binding ExtendsBoundedList
    //- ExtendsBoundedList code ExtendsBoundedBox
    //- ExtendsBoundedBox child.0 ExtendsBoundListType
    //- ExtendsBoundListType.kind "TYPE"
    //- ExtendsBoundListType child.1 ExtendsBoundListTypeArgs
    //- ExtendsBoundListTypeArgs.kind "PARAMETER"
    //- ExtendsBoundListTypeArgs child.0 ExtendsBoundedTypeArgBox
    //- ExtendsBoundedTypeArgBox child.0 ExtendsBoundedTypeArg
    //- ExtendsBoundedTypeArg.pre_text "? extends "
    //- ExtendsBoundedTypeArgBox child.1 TypeExtendsBound
    //- TypeExtendsBound child.1 ObjTypeExtendsBound
    //- ObjTypeExtendsBound.pre_text "Object"
    List<? extends Object> extendsBoundedList;

    //- @superBoundedList defines/binding SuperBoundedList
    //- SuperBoundedList code SuperBoundedBox
    //- SuperBoundedBox child.0 SuperBoundListType
    //- SuperBoundListType.kind "TYPE"
    //- SuperBoundListType child.1 SuperBoundListTypeArgs
    //- SuperBoundListTypeArgs.kind "PARAMETER"
    //- SuperBoundListTypeArgs child.0 SuperBoundedTypeArgBox
    //- SuperBoundedTypeArgBox child.0 SuperBoundedTypeArg
    //- SuperBoundedTypeArg.pre_text "? super "
    //- SuperBoundedTypeArgBox child.1 TypeSuperBound
    //- TypeSuperBound child.1 ObjTypeSuperBound
    //- ObjTypeSuperBound.pre_text "Object"
    List<? super Object> superBoundedList;
  }

  //- @staticTApplyVar defines/binding StaticTApplyVar
  //- StaticTApplyVar typed InnerStaticType
  //- InnerStaticType code TAppCode
  //- TAppCode.kind "TYPE"
  //- TAppCode child.0 TAppCtor
  //- TAppCtor.kind "LOOKUP_BY_PARAM"
  //- TAppCode child.1 TAppParams
  //- TAppParams.kind "PARAMETER_LOOKUP_BY_PARAM"
  //- TAppParams.pre_text "<"
  //- TAppParams.post_text ">"
  //- TAppParams.post_child_text ", "
  //- TAppParams.lookup_index "1"
  static InnerStatic<MarkedSource> staticTApplyVar;

  Object o =
      new Object() {
        //- @field defines/binding AnonField
        //- AnonField childof AnonClass
        //- AnonClass code AnonId
        //- AnonId child.1 AnonCxt
        //- AnonCxt.kind "CONTEXT"
        //- AnonCxt child.0 PkgToken
        //- PkgToken.pre_text pkg
        //- AnonCxt child.1 MksToken
        //- MksToken.pre_text "MarkedSource"
        //- AnonId child.2 AnonToken
        //- AnonToken.kind "IDENTIFIER"
        //- AnonToken.pre_text "(anon 1)"
        int field;
      };

  void methodWithInnerAnon() {
    Object o =
        new Object() {
          //- @field defines/binding IAField
          //- IAField code CIAField
          //- CIAField child.1 CIACxt
          //- CIACxt child.0 CIAPkg
          //- CIACxt child.1 CIAMs
          //- CIACxt child.2 CIAMethod
          //- CIACxt child.3 CIAAnon
          //- !{CIACxt child.4 _}
          //- CIAPkg.pre_text "pkg"
          //- CIAMs.pre_text "MarkedSource"
          //- CIAMethod.pre_text "methodWithInnerAnon"
          //- CIAAnon.pre_text "(anon 2)"
          int field;
        };
  }

  public static class InnerAnon {
    Object o =
        new Object() {
          //- @field defines/binding IIAField
          //- IIAField code CIIAField
          //- CIIAField child.1 CIIACxt
          //- CIIACxt child.2 CIIAInnerCls
          //- CIIACxt child.3 CIIAInnerAnon
          //- CIIAInnerCls.pre_text "InnerAnon"
          //- CIIAInnerAnon.pre_text "(anon 1)"
          int field;
        };
  }

  //- @func defines/binding Func
  //- Func typed FuncType
  //- FuncType code FnTypeCode
  public static Object func() {
    //- @LocalClass defines/binding LocalClass
    //- LocalClass childof Func
    //- LocalClass code LocalClassId
    //- LocalClassId child.1 LocalClassCxt
    //- LocalClassCxt child.0 LocalClassCxtPackage
    //- LocalClassCxtPackage.pre_text "pkg"
    //- LocalClassCtx child.1 LocalClassOuter
    //- LocalClassOuter.kind "IDENTIFIER"
    //- LocalClassOuter.pre_text "MarkedSource"
    //- LocalClassCtx child.2 LocalClassFuncOuter
    //- LocalClassFuncOuter.kind "IDENTIFIER"
    //- LocalClassFuncOuter.pre_text "func"
    //- LocalClassId child.2 LocalClassName
    //- LocalClassName.kind "IDENTIFIER"
    //- LocalClassName.pre_text "LocalClass"
    class LocalClass {}
    return new LocalClass();
  }

  // TODO(schroederc): make verifier emit ArrayTAppCode{,2} with the same VName

  //- @arry defines/binding ArrayParameter
  //- ArrayParameter typed IntArrayArrayType
  //- IntArrayArrayType code ArrayTAppCode
  //- ArrayTAppCode.kind "TYPE"
  //- ArrayTAppCode child.0 ArrayTAppChild
  //- ArrayTAppChild.kind "PARAMETER_LOOKUP_BY_PARAM"
  //- ArrayTAppChild.lookup_index "1"
  //- ArrayTAppChild.post_text "[]"
  //- IntArrayArrayType param.1 IntArrayType
  //- IntArrayType code ArrayTAppCode2
  //- ArrayTAppCode2.kind "TYPE"
  //- ArrayTAppCode2 child.0 ArrayTAppChild2
  //- ArrayTAppChild2.kind "PARAMETER_LOOKUP_BY_PARAM"
  //- ArrayTAppChild2.lookup_index "1"
  //- ArrayTAppChild2.post_text "[]"
  //- IntArrayType param.1 IntType
  //- IntType code IntCode
  //- IntCode.kind "IDENTIFIER"
  //- IntCode.pre_text "int"
  static Object referencesArrayMember(int[][] arry) {
    // No marked source emitted for node defined outside of compilation.
    //- @clone ref ArrayCloneMethod
    //- !{ArrayCloneMethod code _}
    return arry.clone();
  }

  //- @T defines/binding TVar
  //- TVar.node/kind tvar
  //- TVar code TVarCode
  //- TVarCode.kind "BOX"
  //- TVarCode child.0 TVarContext
  //- TVarContext.kind "CONTEXT"
  //- TVarCode child.1 TVarIdent
  //- TVarIdent.kind "IDENTIFIER"
  //- TVarIdent.pre_text "T"
  static class Generic<T> {}

  //- Void code VoidId
  //- VoidId.kind "IDENTIFIER"
  //- VoidId.pre_text void

  //- Int code IntId
  //- IntId.kind "IDENTIFIER"
  //- IntId.pre_text int

  //- String code StringId
  //- StringId.kind "BOX"
  //- StringId child.1 StringCxt
  //- StringCxt.post_child_text "."
  //- StringCxt.add_final_list_token true
  //- StringId child.2 StringIdToken
  //- StringCxt.kind "CONTEXT"
  //- StringCxt child.0 JavaId
  //- JavaId.kind "IDENTIFIER"
  //- JavaId.pre_text java
  //- StringCxt child.1 LangId
  //- LangId.kind "IDENTIFIER"
  //- LangId.pre_text lang
  //- StringIdToken.kind "IDENTIFIER"
  //- StringIdToken.pre_text "String"
}
