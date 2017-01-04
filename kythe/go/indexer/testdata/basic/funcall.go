// Package fun tests basic function call references.
package fun

//- @"func F() {}" defines Fun
//- @F defines/binding Fun
func F() {}

type T struct{}

//- @"func (T) M() {}" defines Meth
//- @M defines/binding Meth = vname("method (fun.T).M", "test", _, "fun", "go")
func (T) M() {}

//- @init defines/binding Init = vname("func init#1", "test", _, "fun", "go")
func init() {
	//- @F ref Fun
	//- @"F()" ref/call Fun
	F()

	var t T

	//- @M ref Meth
	//- @"t.M()" ref/call Meth
	t.M()
}
