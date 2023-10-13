
//- @myFunction defines/binding MyFunction
//- MyFunction code MyFunctionCode
//- 
//- MyFunctionCode child.0 MyFunctionName
//- MyFunctionName.kind "IDENTIFIER"
//- MyFunctionName.pre_text "myFunction"
//-
//- MyFunctionCode child.1 MyFunctionParams
//- MyFunctionParams.kind "PARAMETER_LOOKUP_BY_PARAM"
//- MyFunctionParams.pre_text "("
//- MyFunctionParams.post_text ")"
//- MyFunctionParams.post_child_text ", "
//- 
//- MyFunctionCode child.2 MyFunctionReturnType
//- MyFunctionReturnType.kind "TYPE"
//- MyFunctionReturnType.pre_text ": "
//- MyFunctionReturnType.post_text "number"
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
//- ArgCodeContext child.0 ArgCodeContextId
//- ArgCodeContextId.kind "IDENTIFIER"
//- ArgCodeContextId.pre_text "myFunction"
//-
//- ArgCode child.1 ArgCodeId
//- ArgCodeId.kind "IDENTIFIER"
//- ArgCodeId.pre_text "arg"
//-
//- ArgCode child.2 ArgCodeType
//- ArgCodeType.kind "TYPE"
//- ArgCodeType.pre_text ": "
//- ArgCodeType.post_text "string"
//-
//- ArgCode child.3 ArgCodeDefaultValue
//- ArgCodeDefaultValue.kind "INITIALIZER"
//- ArgCodeDefaultValue.pre_text "'0'"
function myFunction(arg: string = '0'): number {
    return 0;
}

