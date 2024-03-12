//- @+4"docs" defines/binding Pkg
//- @+2"[docs]" ref/doc Pkg

// Package docs is for testing [docs] refs.
package docs

import (
	"fmt"
	"io"
	"io/ioutil"
)

//- @+4"S" defines/binding S
//- @+2"[New]" ref/doc New

// S is referenced by [New]
type S struct{}

//- @+4Clone defines/binding Clone
//- @+2"[*S.Clone]" ref/doc Clone

// Here we have [*S.Clone]
func (S) Clone() *S { return &S{} }

//- @+3"[S]" ref/doc S
//- @+3"New" defines/binding New

// New returns an empty [S].
func New() *S { return new(S) }

//- @+3"[io.EOF]" ref/doc _EOF
//- @+2"[io]" ref/doc _IOPkg=vname("package", _, _, "io", "go")

// Err is just [io.EOF] from the [io] package.
var Err = io.EOF

//- @+3"[ioutil.ReadAll]" ref/doc ReadAll
//- @+2"[io/ioutil.ReadAll]" ref/doc ReadAll

// [ioutil.ReadAll] is short for [io/ioutil.ReadAll]
var _ = ioutil.ReadAll

// [unknown] [pkg.Ident]
const _ = 0

//- @#0+7Stringer defines/binding Stringer
//- @#0+5"[Stringer]" ref/doc Stringer
//- @#1+3"[Stringer]" ref/doc Stringer
//- @#1+2"[Stringer]" ref/doc Stringer

// [Stringer] referenced twice: [Stringer]
// And again on another line: [Stringer]
type Stringer fmt.Stringer
