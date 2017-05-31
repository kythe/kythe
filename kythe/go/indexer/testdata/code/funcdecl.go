// Package funcdecl tests code facts for a function declaration.
package funcdecl

//- @Positive defines/binding Pos
//- Pos code PosCode
//-
//- @x defines/binding Param
//- Param code ParamCode
//-
//- //--------------------------------------------------
//- PosCode child.0 PCFunc   // func
//- PosCode child.1 PCName   // test/funcdecl.Positive
//- PosCode child.2 PCParams // parameters
//- PosCode child.3 PCResult // result
//-
//- //--------------------------------------------------
//- PCFunc.pre_text "func "
//-
//- PCName child.0 PCContext
//- PCName child.1 PCIdent
//-
//- PCParams.kind "PARAMETER_LOOKUP_BY_PARAM"
//- PCParams.pre_text "("
//- PCParams.post_text ")"
//- PCParams.post_child_text ", "
//-
//- PCResult.pre_text " "
//- PCResult child.0 PCReturn
//- PCReturn.pre_text "bool"
//-
//- //--------------------------------------------------
//- PCContext.kind "CONTEXT"
//- PCContext child.0 PCPkg
//- PCPkg.pre_text "funcdecl"
//- PCIdent.kind "IDENTIFIER"
//- PCIdent.pre_text "Positive"
//-
func Positive(x int) bool {
	return x > 0
}

//- @True defines/binding True
//- True code TrueCode
//-
//- TrueCode child.0 TCFunc
//- TrueCode child.1 TCName
//- TrueCode child.2 TCParams
//- TrueCode child.3 TCResult
//-
//- TCFunc.pre_text "func "
//-
//- TCName child.0 TCContext
//- TCName child.1 TCIdent
//-
//- TCParams.kind "PARAMETER"
//- TCParams.pre_text "()"
//-
//- TCResult.pre_text " "
//- TCResult child.0 TCReturn
//- TCReturn.pre_text "bool"
func True() bool { return true}
