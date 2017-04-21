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
	//- FC0.kind "TYPE"
	//- FC0.pre_text "field "
	//-
	//- //--------------------------------------------------
	//- FieldCode child.1 FC1
	//- FC1.kind "BOX"
	//-
	//- FC1 child.0 FC1Context
	//- FC1Context.kind "CONTEXT"
	//- FC1Context.post_child_text "."
	//-
	//- FC1Context child.0 FC1ContextID
	//- FC1ContextID.kind "IDENTIFIER"
	//- FC1ContextID.pre_text "structtype"
	//-
	//- FC1Context child.1 FC1ContextType
	//- FC1ContextType.kind "IDENTIFIER"
	//- FC1ContextType.pre_text "T"
	//-
	//- FC1 child.1 FC1Ident?
	//- FC1Ident.kind "IDENTIFIER"
	//- FC1Ident.pre_text "F"
	//-
	//- //--------------------------------------------------
	//- FieldCode child.2 FC2
	//- FC2.kind "TYPE"
	//- FC2.pre_text " "
	//-
	//- //--------------------------------------------------
	//- FieldCode child.3 FC3
	//- FC3.kind "TYPE"
	//- FC3.pre_text "byte"
	F byte
}
