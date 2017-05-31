// Package structtype tests code facts for a named struct type.
package structtype

//- @T defines/binding Type
//- Type code TypeCode
//-
//- TypeCode.kind "BOX"
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
//- TContext child.0 TPkg
//- TPkg.pre_text "structtype"
//- TIdent.kind "IDENTIFIER"
//- TIdent.pre_text "T"
type T struct {
	//- @F defines/binding Field
	//- Field code FieldCode
	//-
	//- FieldCode.kind "BOX"
	//- FieldCode child.0 FName
	//- FieldCode child.1 FSpace
	//- FieldCode child.2 FType
	//-
	//- FName child.0 FContext
	//- FName child.1 FIdent
	//-
	//- FSpace.pre_text " "
	//-
	//- FType.pre_text "byte"
	//- FType.kind "TYPE"
	//-
	//- FContext.kind "CONTEXT"
	//- FContext child.0 FPkg
	//- FContext child.1 FOwner
	//- FPkg.pre_text "structtype"
	//- FOwner.pre_text "T"
	//- FIdent.kind "IDENTIFIER"
	//- FIdent.pre_text "F"
	F byte
}
