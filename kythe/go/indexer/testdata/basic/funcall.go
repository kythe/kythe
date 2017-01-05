// Package fun tests basic function call references.
package fun

//- File=vname("", "test", _, "src/test/fun/funcall.go", "").node/kind file

//- @F defines/binding Fun = vname("func F", "test", _, "fun", "go")
func F() int { return 0 }

type T struct{}

//- @M defines/binding Meth = vname("method (fun.T).M", "test", _, "fun", "go")
func (p T) M() {}

//- @F ref Fun
//- TCall=@"F()" ref/call Fun
//- TCall childof File
var _ = F()

//- @init defines/binding Init = vname("func init#1", "test", _, "fun", "go")
func init() {
	//- @F ref Fun
	//- FCall=@"F()" ref/call Fun
	//- FCall childof Init
	F()

	var t T

	//- @M ref Meth
	//- MCall=@"t.M()" ref/call Meth
	//- MCall childof Init
	t.M()
}
