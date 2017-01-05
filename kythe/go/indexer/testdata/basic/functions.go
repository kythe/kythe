// Package functions tests function and method structure.
package functions

//- File=vname("", "test", _, "src/test/fun/functions.go", "").node/kind file

//- @"func F(i int) (j int) { return i }" defines Fun
//- @F defines/binding Fun
//- Fun param.0 FPI
func F(i int) (j int) { return i }

type T struct{}

//- @"func (t *T) M(i int) (j int) { return i }" defines Meth
//- @M defines/binding Meth
//- Meth param.0 Recv
//- Meth param.1 MPI
func (t *T) M(i int) (j int) { return i }
