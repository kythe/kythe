
//- @MyClass defines/binding MyClass
//- 
//- MyClass code MyClassCode
//- MyClassCode.kind "IDENTIFIER"
//- MyClassCode.pre_text "MyClass"
class MyClass {
    constructor(arg: string) {}

    //- @myMethod defines/binding MyMethod
    //-
    //- MyMethod code MyMethodCode
    //- MyMethodCode.kind "BOX"
    //-
    //- MyMethodCode child.0 MyMethodContext
    //- MyMethodContext.kind "CONTEXT"
    //- MyMethodContext.pre_text "(method)"
    //-
    //- MyMethodCode child.1 MyMethodSpace
    //- MyMethodSpace.pre_text " "
    //- 
    //- MyMethodCode child.2 MyMethodCodeId
    //- MyMethodCodeId.kind "IDENTIFIER"
    //- MyMethodCodeId.pre_text "myMethod"
    //- 
    //- MyMethodCode child.3 MyMethodParams
    //- MyMethodParams.kind "PARAMETER_LOOKUP_BY_PARAM"
    //- MyMethodParams.pre_text "("
    //- MyMethodParams.post_text ")"
    //- MyMethodParams.post_child_text ", "
    //-
    //- MyMethodCode child.4 MyMethodReturnType
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
    //- ArgCodeContext.pre_text "(parameter)"
    //- 
    //- ArgCode child.1 ArgCodeSpace
    //- ArgCodeSpace.pre_text " "
    //- 
    //- ArgCode child.2 ArgCodeId
    //- ArgCodeId.kind "IDENTIFIER"
    //- ArgCodeId.pre_text "arg"
    //-
    //- ArgCode child.3 ArgCodeType
    //- ArgCodeType.kind "TYPE"
    //- ArgCodeType.pre_text ": "
    //- ArgCodeType.post_text "number"
    myMethod(arg: number): MyClass {
        return this;
    }

    // Test that return type is inferred.
    //- @returnNumber defines/binding ReturnNumber
    //- ReturnNumber code ReturnNumberCode
    //- ReturnNumberCode child.4 ReturnNumberType
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