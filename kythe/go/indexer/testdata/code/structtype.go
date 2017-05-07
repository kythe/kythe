// Package structtype tests code facts for a named struct type.
package structtype

//- @T defines/binding Type
//- Type code TypeCode
//-
//- //--------------------------------------------------
//- TypeCode child.0 TC0
//- TC0.kind "TYPE"
//- TC0.pre_text "type "
//-
//- //--------------------------------------------------
//- TypeCode child.1 TC1
//- TC1.kind "BOX"
//-
//- TC1 child.0 TC1Context
//- TC1Context.kind "CONTEXT"
//- TC1Context.post_child_text "."
//- TC1Context child.0 TC1ContextID
//- TC1ContextID.kind "IDENTIFIER"
//- TC1ContextID.pre_text "structtype"
//-
//- TC1 child.1 TC1Ident
//- TC1Ident.kind "IDENTIFIER"
//- TC1Ident.pre_text "T"
//-
//- //--------------------------------------------------
//- TypeCode child.2 TC2
//- TC2.kind "TYPE"
//- TC2.pre_text " "
//-
//- //--------------------------------------------------
//- TypeCode child.3 TC3
//- TC3.kind "TYPE"
//- TC3.pre_text "struct {...}"
type T struct {
	//- @F defines/binding Field
	//- Field childof Type
	//- Field code FieldCode
	//-
	//- //--------------------------------------------------
	//- FieldCode child.0 FC0
	//- FCo.kind "BOX"
	//-
	//- FC0 child.0 FC1Context
	//- FC0Context.kind "CONTEXT"
	//- FC0Context.post_child_text "."
	//-
	//- FC0Context child.0 FC1ContextID
	//- FC0ContextID.kind "IDENTIFIER"
	//- FC0ContextID.pre_text "structtype"
	//-
	//- FC0Context child.1 FC1ContextType
	//- FC0ContextType.kind "IDENTIFIER"
	//- FC0ContextType.pre_text "T"
	//-
	//- FC0 child.1 FC1Ident?
	//- FC0Ident.kind "IDENTIFIER"
	//- FC0Ident.pre_text "F"
	//-
	//- //--------------------------------------------------
	//- FieldCode child.1 FC1
	//- FC1.kind "TYPE"
	//- FC1.pre_text " "
	//-
	//- //--------------------------------------------------
	//- FieldCode child.2 FC2
	//- FC2.kind "TYPE"
	//- FC2.pre_text "byte"
	F byte
}
