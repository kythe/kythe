// Package funcdecl tests code facts for a function declaration.
package funcdecl

//- @Positive defines/binding Pos
//- Pos code PosCode
//-
//- @x defines/binding Param
//- Param code ParamCode
//-
//- //--------------------------------------------------
//- PosCode child.0 PC0
//- PC0.kind "TYPE"
//- PC0.pre_text "func "
//-
//- //--------------------------------------------------
//- PosCode child.1 PC1
//- PC1 child.0 PC1Context
//- PC1Context.kind "CONTEXT"
//- PC1Context child.0 PC1ContextID
//- PC1ContextID.kind "IDENTIFIER"
//- PC1ContextID.pre_text "funcdecl"
//-
//- PC1 child.1 PC1Ident
//- PC1Ident.kind "IDENTIFIER"
//- PC1Ident.pre_text "Positive"
//-
//- //--------------------------------------------------
//- PosCode child.2 PCParams
//- PCParams.kind "PARAMETER_LOOKUP_BY_PARAM"
//- PCParams.pre_text "("
//- PCParams.post_text ")"
//- PCParams.post_child_text ", "
//-
//- //--------------------------------------------------
//- PosCode child.4 PCResult
//- PCResult.kind "PARAMETER"
//- PCResult child.0 PCBool
//- PCBool.kind "IDENTIFIER"
//- PCBool.pre_text "bool"
func Positive(x int) bool {
	return x > 0
}

//- @True defines/binding True
//- True code TrueCode
//- TrueCode child.2 TC2
//- TC2.pre_text "()"
func True() bool { return true}
