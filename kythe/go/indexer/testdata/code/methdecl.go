// Package methdecl tests code facts for a method declaration.
package methdecl

type w struct{}

// - @LessThan defines/binding LT
// - LT code LTCode
// -
// - LTCode child.0 LTFunc
// - LTCode child.1 LTRecv
// - LTCode child.2 LTName
// - LTCode child.3 LTParams
// - LTCode child.4 LTResult
// -
// - LTFunc.kind "MODIFIER"
// - LTFunc.pre_text "func"
// - LTFunc.post_text " "
// -
// - LTRecv.kind "PARAMETER"
// - LTRecv.pre_text "("
// - LTRecv.post_text ") "
// - LTRecv child.0 LTLookup
// - LTLookup.kind "LOOKUP_BY_PARAM"
// -
// - LTName child.0 LTContext
// - LTName child.1 LTIdent
// -
// - LTParams.kind "PARAMETER_LOOKUP_BY_PARAM"
// - LTParams.lookup_index 1
// - LTParams.pre_text "("
// - LTParams.post_text ")"
// - LTParams.post_child_text ", "
// -
// - LTResult.pre_text " "
// - LTResult.kind "TYPE"
// - LTResult child.0 LTReturn
// - LTReturn.pre_text "bool"
// -
// - LTContext.kind "CONTEXT"
// - LTContext.post_child_text "."
// - LTContext child.0 LTPkg
// - LTContext child.1 LTCType
// - LTPkg.pre_text "methdecl"
// - LTCType.pre_text "w"
// - LTIdent.pre_text "LessThan"
// -
// - @x defines/binding LTX
// - LTX code XCode
// - XCode child.0 XName
// - XName child.0 XCtx
// - XCtx.kind "CONTEXT"
// - XCtx child.0 XPkg
// - XCtx child.1 XRec
// - XCtx child.2 XFun
// - XName child.1 XId
// - XPkg.kind "IDENTIFIER"
// - XPkg.pre_text "methdecl"
// - XRec.kind "IDENTIFIER"
// - XRec.pre_text "w"
// - XFun.kind "IDENTIFIER"
// - XFun.pre_text "LessThan"
// - XId.kind "IDENTIFIER"
// - XId.pre_text "x"
// -
func (rec *w) LessThan(x int) bool {
	return x < 0
}

type decorCommand struct{}
type Context any
type FlagSet struct{}
type API struct{}

// - @Run defines/binding RunFunc
// - RunFunc code RFCode
// -
// - RFCode child.0 RFFunc
// - RFCode child.1 RFRecv
// - RFCode child.2 RFName
// - RFCode child.3 RFParams
// - RFCode child.4 RFResult
// -
// - RFFunc.kind "MODIFIER"
// - RFFunc.pre_text "func"
// - RFFunc.post_text " "
// -
// - RFRecv.kind "PARAMETER"
// - RFRecv.pre_text "("
// - RFRecv.post_text ") "
// - RFRecv child.0 RFLookup
// - RFLookup.kind "LOOKUP_BY_PARAM"
// -
// - RFName child.0 RFContext
// - RFName child.1 RFIdent
// -
// - RFParams.kind "PARAMETER_LOOKUP_BY_PARAM"
// - RFParams.lookup_index 1
// - RFParams.pre_text "("
// - RFParams.post_text ")"
// - RFParams.post_child_text ", "
// -
// - RFResult.pre_text " "
// - RFResult.kind "TYPE"
// - RFResult child.0 RFReturn
// - RFReturn.pre_text "error"
// -
// - RFContext.kind "CONTEXT"
// - RFContext.post_child_text "."
// - RFContext child.0 RFPkg
// - RFContext child.1 RFCType
// - RFPkg.pre_text "methdecl"
// - RFCType.pre_text "decorCommand"
// - RFIdent.pre_text "Run"
// -
// - @ctx defines/binding RFCtx
// - RFCtx code CtxCode
// - CtxCode child.0 CtxName
// - CtxCode child.1 CtxType
// - CtxName child.0 CtxCtx
// - CtxCtx.kind "CONTEXT"
// - CtxCtx child.0 CtxPkg
// - CtxCtx child.1 CtxRec
// - CtxCtx child.2 CtxFun
// - CtxName child.1 CtxId
// - CtxPkg.kind "IDENTIFIER"
// - CtxPkg.pre_text "methdecl"
// - CtxRec.kind "IDENTIFIER"
// - CtxRec.pre_text "decorCommand"
// - CtxFun.kind "IDENTIFIER"
// - CtxFun.pre_text "Run"
// - CtxId.kind "IDENTIFIER"
// - CtxId.pre_text "ctx"
// - CtxType.kind "LOOKUP_BY_TYPED"
// - CtxType.lookup_index 0
// -
// - @Context ref CtxTypeValue
// - CtxTypeValue code CtxTypeValueCode
// - CtxTypeValueCode child.0 CtxTypeValueCodeCtx
// - CtxTypeValueCodeCtx.kind "CONTEXT"
// - CtxTypeValueCodeCtx.post_child_text "."
// - CtxTypeValueCodeCtx child.0 CtxTypeValueCodeCtxChild
// - CtxTypeValueCodeCtxChild.kind "IDENTIFIER"
// - CtxTypeValueCodeCtxChild.pre_text "methdecl"
// - CtxTypeValueCode child.1 CtxTypeValueCodeID
// - CtxTypeValueCodeID.kind "IDENTIFIER"
// - CtxTypeValueCodeID.pre_text "Context"
// -
// - @flag defines/binding RFFlag
// - RFFlag code FlagCode
// - FlagCode child.0 FlagName
// - FlagName child.0 FlagCtx
// - FlagCtx.kind "CONTEXT"
// - FlagCtx child.0 FlagPkg
// - FlagCtx child.1 FlagRec
// - FlagCtx child.2 FlagFun
// - FlagName child.1 FlagId
// - FlagPkg.kind "IDENTIFIER"
// - FlagPkg.pre_text "methdecl"
// - FlagRec.kind "IDENTIFIER"
// - FlagRec.pre_text "decorCommand"
// - FlagFun.kind "IDENTIFIER"
// - FlagFun.pre_text "Run"
// - FlagId.kind "IDENTIFIER"
// - FlagId.pre_text "flag"
// -
// - @FlagSet ref FlagTypeValue
// - FlagTypeValue code FlagTypeValueCode
// - FlagTypeValueCode child.0 FlagTypeValueCodeCtx
// - FlagTypeValueCode child.1 FlagTypeValueCodeID
// - FlagTypeValueCodeCtx.kind "CONTEXT"
// - FlagTypeValueCodeCtx.post_child_text "."
// - FlagTypeValueCodeCtx child.0 FlagTypeValueCodeCtxChild
// - FlagTypeValueCodeCtxChild.kind "IDENTIFIER"
// - FlagTypeValueCodeCtxChild.pre_text "methdecl"
// - FlagTypeValueCodeID.kind "IDENTIFIER"
// - FlagTypeValueCodeID.pre_text "FlagSet"
// -
// - @api defines/binding RFApi
// - RFApi code ApiCode
// - ApiCode child.0 ApiName
// - ApiName child.0 ApiCtx
// - ApiCtx.kind "CONTEXT"
// - ApiCtx child.0 ApiPkg
// - ApiCtx child.1 ApiRec
// - ApiCtx child.2 ApiFun
// - ApiName child.1 ApiId
// - ApiPkg.kind "IDENTIFIER"
// - ApiPkg.pre_text "methdecl"
// - ApiRec.kind "IDENTIFIER"
// - ApiRec.pre_text "decorCommand"
// - ApiFun.kind "IDENTIFIER"
// - ApiFun.pre_text "Run"
// - ApiId.kind "IDENTIFIER"
// - ApiId.pre_text "api"
// -
// - @API ref ApiTypeValue
// - ApiTypeValue code ApiTypeValueCode
// - ApiTypeValueCode child.0 ApiTypeValueCodeCtx
// - ApiTypeValueCode child.1 ApiTypeValueCodeID
// - ApiTypeValueCodeCtx.kind "CONTEXT"
// - ApiTypeValueCodeCtx.post_child_text "."
// - ApiTypeValueCodeCtx child.0 ApiTypeValueCodeCtxChild
// - ApiTypeValueCodeCtxChild.kind "IDENTIFIER"
// - ApiTypeValueCodeCtxChild.pre_text "methdecl"
// - ApiTypeValueCodeID.kind "IDENTIFIER"
// - ApiTypeValueCodeID.pre_text "API"
func (c decorCommand) Run(ctx Context, flag *FlagSet, api API) error {
	return nil
}
