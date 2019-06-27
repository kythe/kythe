// Package structtype tests code facts for a named struct type.
package structtype

//- @T defines/binding Type
//- Type code TypeCode
//-
//- TypeCode.kind "BOX"
//- TypeCode.post_child_text " "
//- TypeCode child.0 TType
//- TypeCode child.1 TName
//- TypeCode child.2 TStruct
//-
//- TType.pre_text "type"
//-
//- TName child.0 TContext
//- TName child.1 TIdent
//-
//- TStruct.pre_text "struct {...}"
//- TStruct.kind "TYPE"
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
	//- FieldCode.post_child_text " "
	//- FieldCode child.0 FName
	//- FieldCode child.1 FType
	//-
	//- FName child.0 FContext
	//- FName child.1 FIdent
	//-
	//- FType.kind "LOOKUP_BY_TYPED"
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
