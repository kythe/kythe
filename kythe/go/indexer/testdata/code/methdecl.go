// Package methdecl tests code facts for a method declaration.
package methdecl

type w int

//- @LessThan defines/binding LT
//- LT code LTCode
//-
//- LTCode child.0 LTFunc
//- LTCode child.1 LTRecv
//- LTCode child.2 LTName
//- LTCode child.3 LTParams
//- LTCode child.4 LTResult
//-
//- LTFunc.pre_text "func "
//-
//- LTRecv.kind "PARAMETER"
//- LTRecv.pre_text "("
//- LTRecv.post_text ") "
//- LTRecv child.0 LTRType
//-
//- LTName child.0 LTContext
//- LTName child.1 LTIdent
//-
//- LTParams.kind "PARAMETER_LOOKUP_BY_PARAM"
//- LTParams.lookup_index 1
//- LTParams.pre_text "("
//- LTParams.post_text ")"
//- LTParams.post_child_text ", "
//-
//- LTResult.pre_text " "
//- LTResult child.0 LTReturn
//- LTReturn.pre_text "bool"
//-
//- LTRType.kind "TYPE"
//- LTRType.pre_text "w"
//-
//- LTContext.kind "CONTEXT"
//- LTContext.post_child_text "."
//- LTContext child.0 LTPkg
//- LTPkg.pre_text "methdecl"
//- LTIdent.pre_text "LessThan"
func (rec w) LessThan(x int) bool {
	return int(rec) < x
}
