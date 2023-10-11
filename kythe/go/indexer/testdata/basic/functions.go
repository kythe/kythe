// Package functions tests function and method structure.
package functions

//- Pkg=vname("package", "test", _, "fun", "go").node/kind package
//- Init=vname("package.<init>@59", "test", _, "fun", "go").node/kind function
//- Init childof Pkg

// Anonymous functions do not get binding anchors.
//
// - @"func(b bool) {}" defines
// -   Anon1 = vname("package.<init>@59$1", "test", _, "fun", "go")
// - Anon1 param.0 _AnonPB
// - Anon1.node/kind function
var _ = func(b bool) {}

// - @"func(z int) {}" defines
// -   Anon2 = vname("package.<init>@59$2", "test", _, "fun", "go")
// - Anon2 param.0 _AnonPZ
// - Anon2.node/kind function
var _ = func(z int) {}

// - @"func F(input int) (output int) { return 17 }" defines Fun
// - @F defines/binding Fun
// -
// - @input defines/binding FunInput
// - FunInput.node/kind variable
// - FunInput.subkind local/parameter
// - Fun param.0 FunInput
// - FunInput childof Fun
// -
// - @output defines/binding FunOutput
// - FunOutput childof Fun
func F(input int) (output int) { return 17 }

type T struct{}

// - @"func (recv *T) M(input int) (output int) { return 34 }" defines Meth
// - @M defines/binding Meth
// -
// - @recv defines/binding Recv
// - Recv.node/kind variable
// - Meth param.0 Recv
// - Recv childof Meth
// -
// - @input defines/binding MethInput
// - MethInput.node/kind variable
// - Meth param.1 MethInput
// - MethInput childof Meth
// -
// - @output defines/binding MethOutput
// - MethOutput.node/kind variable
// - MethOutput childof Meth
// - Meth childof _Struct
func (recv *T) M(input int) (output int) { return 34 }

// - @outer defines/binding Outer
// - Outer.node/kind function
func outer() {
	//- @"func(q bool) {}" defines Inner = vname("func outer$1", _, _, _, _)
	//- Inner param.0 _InnerPQ
	//- Inner.node/kind function
	_ = func(q bool) {}
}

// - @ignore defines/binding Ignore
// - Ignore param.0 Unnamed
// - Unnamed.node/kind variable
// - !{_X defines/binding Unnamed}
func ignore(_ int) bool { return false }
