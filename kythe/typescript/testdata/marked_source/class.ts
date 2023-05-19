
//- @MyClass defines/binding MyClass
//- 
//- MyClass code MyClassCode
//- MyClassCode.kind "IDENTIFIER"
//- MyClassCode.pre_text "MyClass"
class MyClass {
    //- @constructor defines/binding Constructor
    //-
    //- Constructor code ConstructorCode
    //- ConstructorCode.kind "BOX"
    //-
    //- ConstructorCode child.0 ConstructorContext
    //- ConstructorContext.kind "CONTEXT"
    //- ConstructorContext.post_child_text "."
    //- ConstructorContext.add_final_list_token true
    //-
    //- ConstructorContext child.0 ConstructorContextId
    //- ConstructorContextId.kind "IDENTIFIER"
    //- ConstructorContextId.pre_text "MyClass"
    //-
    //- ConstructorCode child.1 ConstructorId
    //- ConstructorId.kind "IDENTIFIER"
    //- ConstructorId.pre_text "constructor"
    //-
    //- ConstructorCode child.2 ConstructorParams
    //- ConstructorParams.kind "PARAMETER_LOOKUP_BY_PARAM"
    //- ConstructorParams.pre_text "("
    //- ConstructorParams.post_text ")"
    //- ConstructorParams.post_child_text ", "
    constructor(arg: string) {}

    //- @myMethod defines/binding MyMethod
    //-
    //- MyMethod code MyMethodCode
    //- MyMethodCode.kind "BOX"
    //-
    //- MyMethodCode child.0 MyMethodContext
    //- MyMethodContext.kind "CONTEXT"
    //- MyMethodContext.post_child_text "."
    //- MyMethodContext.add_final_list_token true
    //-
    //- MyMethodContext child.0 MyMethodContextId
    //- MyMethodContextId.kind "IDENTIFIER"
    //- MyMethodContextId.pre_text "MyClass"
    //-
    //- MyMethodCode child.1 MyMethodCodeId
    //- MyMethodCodeId.kind "IDENTIFIER"
    //- MyMethodCodeId.pre_text "myMethod"
    //- 
    //- MyMethodCode child.2 MyMethodParams
    //- MyMethodParams.kind "PARAMETER_LOOKUP_BY_PARAM"
    //- MyMethodParams.pre_text "("
    //- MyMethodParams.post_text ")"
    //- MyMethodParams.post_child_text ", "
    //-
    //- MyMethodCode child.3 MyMethodReturnType
    //- MyMethodReturnType.kind "TYPE"
    //- MyMethodReturnType.pre_text ": "
    //- MyMethodReturnType.post_text "MyClass"
    //- 
    //- @arg defines/binding Arg
    //- Arg code ArgCode
    //- ArgCode.kind "BOX"
    //- 
    //- ArgCode child.0 ArgCodeContext
    //- ArgCodeContext.kind "CONTEXT"
    //- ArgCodeContext.post_child_text "."
    //- ArgCodeContext.add_final_list_token true
    //-
    //- ArgCodeContext child.0 ArgCodeContextClassId
    //- ArgCodeContextClassId.kind "IDENTIFIER"
    //- ArgCodeContextClassId.pre_text "MyClass"
    //-
    //- ArgCodeContext child.1 ArgCodeContextMethodId
    //- ArgCodeContextMethodId.kind "IDENTIFIER"
    //- ArgCodeContextMethodId.pre_text "myMethod"
    //- 
    //- ArgCode child.1 ArgCodeId
    //- ArgCodeId.kind "IDENTIFIER"
    //- ArgCodeId.pre_text "arg"
    //-
    //- ArgCode child.2 ArgCodeType
    //- ArgCodeType.kind "TYPE"
    //- ArgCodeType.pre_text ": "
    //- ArgCodeType.post_text "number"
    myMethod(arg: number): MyClass {
        return this;
    }

    // Test that return type is inferred.
    //- @returnNumber defines/binding ReturnNumber
    //- ReturnNumber code ReturnNumberCode
    //- ReturnNumberCode child.3 ReturnNumberType
    //- ReturnNumberType.kind "TYPE"
    //- ReturnNumberType.pre_text ": "
    //- ReturnNumberType.post_text "number"
    returnNumber() {
        return 42;
    }
}

//- @MyInterface defines/binding MyInterface
//-
//- MyInterface code MyInterfaceCode
//- MyInterfaceCode.kind "IDENTIFIER"
//- MyInterfaceCode.pre_text "MyInterface"
interface MyInterface {}