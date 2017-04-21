// Package pkgvar tests code facts for package-level variables.
package pkgvar

//- @V defines/binding Var
//- Var code VarCode
//-
//- //--------------------------------------------------
//- VarCode child.0 VC0
//- VC0.kind "TYPE"
//- VC0.pre_text "var "
//-
//- //--------------------------------------------------
//- VarCode child.1 VC1
//- VC1.kind "BOX"
//-
//- VC1 child.0 VC1Context
//- VC1Context.kind "CONTEXT"
//- VC1Context.post_child_text "."
//- VC1Context child.0 VC1ContextID
//- VC1ContextID.kind "IDENTIFIER"
//- VC1ContextID.pre_text "pkgvar"
//-
//- VC1 child.1 VC1Ident
//- VC1Ident.kind "IDENTIFIER"
//- VC1Ident.pre_text "V"
//-
//- //--------------------------------------------------
//- VarCode child.2 VC2
//- VC2.kind "TYPE"
//- VC2.pre_text " "
//-
//- //--------------------------------------------------
//- VarCode child.3 VC3
//- VC3.kind "TYPE"
//- VC3.pre_text "int"
var V int
