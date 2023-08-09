
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
    //- MyValueContext.post_child_text "."
    //- MyValueContext.add_final_list_token true
    //-
    //- MyValueContext child.0 MyValueContextId
    //- MyValueContextId.kind "IDENTIFIER"
    //- MyValueContextId.pre_text "MyEnum"
    //-
    //- MyValueCode child.1 MyValueId
    //- MyValueId.kind "IDENTIFIER"
    //- MyValueId.pre_text "MY_VALUE"
    //-
    //- MyValueCode child.2 MyValueInitializer
    //- MyValueInitializer.kind "INITIALIZER"
    //- MyValueInitializer.pre_text "123"
    MY_VALUE = 123,
}
