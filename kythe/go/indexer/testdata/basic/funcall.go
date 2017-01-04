// Package fun tests basic function call references.
package fun

func F() {}

type T struct{}

func (T) M() {}

func init() {
	//- @F ref Fun
	//- @"F()" ref/call Fun
	F()

	var t T

	//- @M ref Meth
	//- @"t.M()" ref/call Meth
	t.M()
}
