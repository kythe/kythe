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

//- @"func F(input int) (output int) { return 17 }" defines Fun
//- @F defines/binding Fun
//-
//- @input defines/binding FunInput
//- Fun param.0 FunInput
//- FunInput childof Fun
//-
//- @output defines/binding FunOutput
//- FunOutput childof Fun
func F(input int) (output int) { return 17 }

type T struct{}

//- @"func (recv *T) M(input int) (output int) { return 34 }" defines Meth
//- @M defines/binding Meth
//-
//- @recv defines/binding Recv
//- Recv.node/kind variable
//- Meth param.0 Recv
//- Recv childof Meth
//-
//- @input defines/binding MethInput
//- MethInput.node/kind variable
//- Meth param.1 MethInput
//- MethInput childof Meth
//-
//- @output defines/binding MethOutput
//- MethOutput.node/kind variable
//- MethOutput childof Meth
//- Meth childof Struct
func (recv *T) M(input int) (output int) { return 34 }

//- @outer defines/binding Outer
//- Outer.node/kind function
func outer() {
	//- @"func(q bool) {}" defines Inner = vname("func outer$1", _, _, _, _)
	//- Inner param.0 InnerPQ
	_ = func(q bool) {}
}

//- @ignore defines/binding Ignore
//- Ignore param.0 Unnamed
//- Unnamed.node/kind variable
//- !{X defines/binding Unnamed}
func ignore(_ int) bool { return false }
