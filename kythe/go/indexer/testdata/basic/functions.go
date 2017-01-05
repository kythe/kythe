// Package functions tests function and method structure.
package functions

//- Pkg=vname("package", "test", _, "fun", "go").node/kind package
//- Init=vname("package.<init>", "test", _, "fun", "go").node/kind function
//- Init childof Pkg

// Anonymous functions do not get binding anchors.
//
//- @"func(b bool) {}" defines
//-   Anon = vname("package.<init>$1", "test", _, "fun", "go")
//- Anon param.0 AnonPB
var _ = func(b bool) {}

//- @"func F(i int) (j int) { return i }" defines Fun
//- @F defines/binding Fun
//- Fun param.0 FunPI
func F(i int) (j int) { return i }

type T struct{}

//- @"func (t *T) M(i int) (j int) { return i }" defines Meth
//- @M defines/binding Meth
//- Meth param.0 Recv
//- Meth param.1 MethPI
//- Meth childof Struct
func (t *T) M(i int) (j int) { return i }

//- @outer defines/binding Outer
func outer() {
	//- @"func(q bool) {}" defines Inner
	//- Inner param.0 InnerPQ
	_ = func(q bool) {}
}
