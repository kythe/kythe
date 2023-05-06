
//- @MyEnum defines/binding MyEnum
//-
//- MyEnum code MyEnumCode
//- MyEnumCode.kind "IDENTIFIER"
//- MyEnumCode.pre_text "MyEnum"
enum MyEnum {

    //- @MY_VALUE defines/binding MyValue
    //-
    //- MyValue code MyValueCode
    //- MyValueCode.kind "BOX"
    //- 
    //- MyValueCode child.0 MyValueContext
    //- MyValueContext.kind "CONTEXT"
    //- MyValueContext.pre_text "MyEnum"
    //-
    //- MyValueCode child.1 MyValueSpace
    //- MyValueSpace.pre_text " "
    //-
    //- MyValueCode child.2 MyValueId
    //- MyValueId.kind "IDENTIFIER"
    //- MyValueId.pre_text "MY_VALUE"
    //- 
    //- MyValueCode child.3 MyValueEqual
    //- MyValueEqual.kind "BOX"
    //- MyValueEqual.pre_text " = "
    //-
    //- MyValueCode child.4 MyValueInitializer
    //- MyValueInitializer.kind "INITIALIZER"
    //- MyValueInitializer.pre_text "123"
    MY_VALUE = 123,
}