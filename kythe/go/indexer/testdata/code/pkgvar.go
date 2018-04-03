// Package pkgvar tests code facts for package-level variables.
package pkgvar

//- @V defines/binding Var
//- Var code VarCode
//-
//- VarCode child.0 VName
//- VarCode child.1 VType
//- VarCode.post_child_text " "
//-
//- VName child.0 VContext
//- VName child.1 VIdent
//-
//- VType.pre_text "int"
//-
//- VContext.kind "CONTEXT"
//- VContext child.0 VPkg
//- VPkg.pre_text "pkgvar"
//-
//- VIdent.kind "IDENTIFIER"
//- VIdent.pre_text "V"
var V int
