// Package pkgvar tests code facts for package-level variables.
package pkgvar

//- @V defines/binding Var
//- Var code VarCode
//-
//- //--------------------------------------------------
//- VarCode child.0 VC0
//- VC0.kind "BOX"
//-
//- VC0 child.0 VC1Context
//- VC0Context.kind "CONTEXT"
//- VC0Context.post_child_text "."
//- VC0Context child.0 VC1ContextID
//- VC0ContextID.kind "IDENTIFIER"
//- VC0ContextID.pre_text "pkgvar"
//-
//- VC0 child.1 VC1Ident
//- VC0Ident.kind "IDENTIFIER"
//- VC0Ident.pre_text "V"
//-
//- //--------------------------------------------------
//- VarCode child.1 VC1
//- VC1.kind "TYPE"
//- VC1.pre_text " "
//-
//- //--------------------------------------------------
//- VarCode child.2 VC2
//- VC2.kind "TYPE"
//- VC2.pre_text "int"
var V int
