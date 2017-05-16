// Package structtype tests code facts for a named struct type.
package structtype

//- @T defines/binding Type
//- Type code TypeCode
//-
//- TypeCode.kind "TYPE"
//- TypeCode child.0 TType
//- TypeCode child.1 TName
//- TypeCode child.2 TSpace
//- TypeCode child.3 TStruct
//-
//- TType.pre_text "type "
//-
//- TName child.0 TContext
//- TName child.1 TIdent
//-
//- TSpace.pre_text " "
//-
//- TStruct.pre_text "struct {...}"
//-
//- TContext.kind "CONTEXT"
//- TContext.pre_text "structtype"
//- TIdent.kind "IDENTIFIER"
//- TIdent.pre_text "T"
type T struct {
	//- @F defines/binding Field
	//- Field code FieldCode
	//-
	//- FieldCode.kind "TYPE"
	//- FieldCode child.0 FName
	//- FieldCode child.1 FSpace
	//- FieldCode child.2 FType
	//-
	//- FName child.0 FContext
	//- FName child.1 FOwner
	//- FName child.2 FIdent
	//-
	//- FSpace.pre_text " "
	//-
	//- FType.pre_text "byte"
	//-
	//- FContext.kind "CONTEXT"
	//- FContext.pre_text "structtype"
	//- FOwner.kind "CONTEXT"
	//- FOwner.pre_text "T"
	//- FIdent.kind "IDENTIFIER"
	//- FIdent.pre_text "F"
	F byte
}
