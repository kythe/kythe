// Package pkgvar tests code facts for package-level variables.
package pkgvar

//- @V defines/binding Var
//- Var code VarCode
//-
//- VarCode child.0 VName
//- VarCode child.1 VSpace
//- VarCode child.2 VType
//-
//- VName child.0 VContext
//- VName child.1 VIdent
//-
//- VSpace.pre_text " "
//- VType.pre_text "int"
//-
//- VContext.kind "CONTEXT"
//- VContext child.0 VPkg
//- VPkg.pre_text "pkgvar"
//-
//- VIdent.kind "IDENTIFIER"
//- VIdent.pre_text "V"
var V int
