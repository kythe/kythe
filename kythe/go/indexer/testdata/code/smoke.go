// Package smoke verifies that basic code facts are emitted for nodes that
// support them.
//
// TODO(fromberger): Expand these tests once we figure out what to do about
// destructuring type signatures.
package smoke

//- @V defines/binding Var
//- Var code VarCode
//- VarCode.pre_text "var smoke.V int"
var V int

//- @T defines/binding Type
//- Type code TypeCode
//- TypeCode.pre_text "type smoke.T struct{F byte}"
type T struct{
	//- @F defines/binding Field
	//- Field childof Type
	//- Field code FieldCode?
	//- FieldCode.pre_text "field F byte"
	F byte
}

//- @Positive defines/binding Func
//- Func code FuncCode
//-
//- @x defines/binding Param
//- Param code ParamCode
//-
//- FuncCode.pre_text "func smoke.Positive(x int) bool"
//- ParamCode.pre_text "var x int"
func Positive(x int) bool {
	return x > 0
}
