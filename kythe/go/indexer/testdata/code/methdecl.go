// Package methdecl tests code facts for a method declaration.
package methdecl

type w int

//- @LessThan defines/binding LT
//- LT code LTCode
//-
//- @rec defines/binding Rec
//- Rec code RecCode
//-
//- @x defines/binding Param
//- Param code ParamCode
//-
//- //--------------------------------------------------
//- LTCode child.0 LC0
//- LC0.kind "TYPE"
//- LC0.pre_text "func "
//-
//- //--------------------------------------------------
//- LTCode child.1 LRec
//- LRec.kind "PARAMETER"
//- LRec.pre_text "("
//- LRec.post_text ") "
//- LRec child.0 LRectype
//- LRecType.kind "TYPE"
//- LRecType.pre_text "w"
//-
//- //--------------------------------------------------
//- LTCode child.2 LC2
//- LC2 child.0 LC2Context
//- LC2Context.kind "CONTEXT"
//- LC2Context child.0 LC2ContextID
//- LC2ContextID.kind "IDENTIFIER"
//- LC2ContextID.pre_text "methdecl"
//-
//- LC2 child.1 LC2Ident
//- LC2Ident.kind "IDENTIFIER"
//- LC2Ident.pre_text "LessThan"
//-
//- //--------------------------------------------------
//- LTCode child.3 LCParams
//- LCParams.kind "PARAMETER_LOOKUP_BY_PARAM"
//- LCParams.pre_text "("
//- LCParams.post_text ")"
//- LCParams.post_child_text ", "
//- LCParams.lookup_index 1
//-
//- //--------------------------------------------------
//- LTCode child.5 LCResult
//- LCResult.kind "PARAMETER"
//- LCResult child.0 LCBool
//- LCBool.kind "IDENTIFIER"
//- LCBool.pre_text "bool"
func (rec w) LessThan(x int) bool {
	return int(rec) < x
}
