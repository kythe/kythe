// Package methdecl tests code facts for a method declaration.
package methdecl

type w struct{}

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
//- LTResult.kind "TYPE"
//- LTResult child.0 LTReturn
//- LTReturn.pre_text "bool"
//-
//- LTRType.kind "TYPE"
//- LTRType.pre_text "*w"
//-
//- LTContext.kind "CONTEXT"
//- LTContext.post_child_text "."
//- LTContext child.0 LTPkg
//- LTContext child.1 LTCType
//- LTPkg.pre_text "methdecl"
//- LTCType.pre_text "w"
//- LTIdent.pre_text "LessThan"
//-
//- @x defines/binding LTX
//- LTX code XCode
//- XCode child.0 XName
//- XName child.0 XCtx
//- XCtx.kind "CONTEXT"
//- XCtx child.0 XPkg
//- XCtx child.1 XRec
//- XCtx child.2 XFun
//- XName child.1 XId
//- XPkg.kind "IDENTIFIER"
//- XPkg.pre_text "methdecl"
//- XRec.kind "IDENTIFIER"
//- XRec.pre_text "w"
//- XFun.kind "IDENTIFIER"
//- XFun.pre_text "LessThan"
//- XId.kind "IDENTIFIER"
//- XId.pre_text "x"
//-
func (rec *w) LessThan(x int) bool {
	return x < 0
}
