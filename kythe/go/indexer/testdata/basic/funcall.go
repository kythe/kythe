// Package fun tests basic function call references.
//- @fun defines/binding Pkg
package fun

//- Pkg.node/kind package
//- Init childof Pkg
//- Init.node/kind function

//- @F defines/binding Fun = vname("func F", "test", _, "fun", "go")
func F() int { return 0 }

type T struct{}

//- @M defines/binding Meth = vname("method (fun.T).M", "test", _, "fun", "go")
func (p T) M() {}

//- @F ref Fun
//- TCall=@"F()" ref/call Fun
//- TCall childof Init
var _ = F()

//- @init defines/binding InitFunc = vname("func init#1", "test", _, "fun", "go")
func init() {
	//- @F ref Fun
	//- FCall=@"F()" ref/call Fun
	//- FCall childof InitFunc
	F()

	var t T

	//- @M ref Meth
	//- MCall=@"t.M()" ref/call Meth
	//- MCall childof InitFunc
	t.M()
}
