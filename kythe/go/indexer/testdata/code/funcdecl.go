// Package funcdecl tests code facts for a function declaration.
// - @funcdecl defines/binding Pkg
package funcdecl

// - @Positive defines/binding Pos
// - Pos code PosCode
// -
// - @x defines/binding Param  // see Note [1].
// - Param code XCode
// - XCode.post_child_text " "
// - XCode child.0 XName
// - XCode child.1 XTypeLookup
// - XTypeLookup.kind "LOOKUP_BY_TYPED"
// - Param typed ParamType
// - ParamType code XType
// -
// - XName child.0 XCtx
// - XName child.1 XId
// -
// - XCtx child.0 XPkg
// - XCtx child.1 XFunc
// - XPkg.kind "IDENTIFIER"
// - XPkg.pre_text "funcdecl"
// - XPkg link Pkg
// - XFunc.kind "IDENTIFIER"
// - XFunc.pre_text "Positive"
// - XId.kind "IDENTIFIER"
// - XId.pre_text "x"
// -
// - XType.kind "TYPE"
// - XType.pre_text "int"
// -
// - //--------------------------------------------------
// - PosCode child.0 PCFunc   // func
// - PosCode child.1 PCName   // test/funcdecl.Positive
// - PosCode child.2 PCParams // parameters
// - PosCode child.3 PCResult // result
// -
// - //--------------------------------------------------
// - PCFunc.kind "MODIFIER"
// - PCFunc.pre_text "func"
// - PCFunc.post_text " "
// -
// - PCName child.0 PCContext
// - PCName child.1 PCIdent
// -
// - PCParams.kind "PARAMETER_LOOKUP_BY_PARAM"
// - PCParams.pre_text "("
// - PCParams.post_text ")"
// - PCParams.post_child_text ", "
// -
// - PCResult.pre_text " "
// - PCResult child.0 PCReturn
// - PCReturn.pre_text "bool"
// -
// - //--------------------------------------------------
// - PCContext.kind "CONTEXT"
// - PCContext child.0 PCPkg
// - PCPkg.pre_text "funcdecl"
// - PCPkg link Pkg
// - PCIdent.kind "IDENTIFIER"
// - PCIdent.pre_text "Positive"
// - PCIdent link Pos
func Positive(x int) bool {
	return x > 0
}

// Note [1]: The occurrence of the signature for x inside the marked source for
// the function will be looked up by the server (or denormalized in post). This
// checks that the explicit signature is generated correctly in situ.

// - @True defines/binding True
// - True code TrueCode
// -
// - TrueCode child.0 TCFunc
// - TrueCode child.1 TCName
// - TrueCode child.2 TCParams
// - TrueCode child.3 TCResult
// -
// - TCFunc.kind "MODIFIER"
// - TCFunc.pre_text "func"
// - TCFunc.post_text " "
// -
// - TCName child.0 _TCContext
// - TCName child.1 _TCIdent
// -
// - TCParams.kind "PARAMETER"
// - TCParams.pre_text "()"
// -
// - TCResult.pre_text " "
// - TCResult child.0 TCReturn
// - TCReturn.pre_text "bool"
func True() bool { return true }

// - @N defines/binding N
// - N code NCode
// - NCode child.3 Ret
// - Ret.pre_text " ("
// - Ret.post_child_text ", "
// - Ret.post_text ")"
// - Ret child.0 ErrRet
// - ErrRet.post_child_text " "
// - ErrRet child.0 ErrVar
// - ErrVar.pre_text "err"
// - ErrVar link Err
// - ErrRet child.1 ErrType
// - ErrType.pre_text "error"
// - @#0err defines/binding Err
func N() (err error) { return nil }
