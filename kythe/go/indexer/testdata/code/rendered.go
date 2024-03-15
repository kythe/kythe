package rendered

import (
	"fmt"
	"math/rand"
)

// - @F defines/binding F
// - F.code/rendered/qualified_name "rendered.F"
// - F.code/rendered/callsite_signature "F()"
func F() {}

// - @S defines/binding S
// - S.code/rendered/qualified_name "rendered.S"
// - S.code/rendered/signature "type S"
type S struct{}

// - @M defines/binding M
// - M.code/rendered/qualified_name "rendered.S.M"
// - M.code/rendered/signature "func (s *S) M()"
// - M.code/rendered/callsite_signature "(s) M()"
func (s *S) M() {}

// - @MArg defines/binding MArg
// - MArg.code/rendered/qualified_name "rendered.S.MArg"
// - MArg.code/rendered/callsite_signature "(s) MArg(arg)"
// - @arg defines/binding Arg
// - Arg.code/rendered/qualified_name "rendered.S.MArg.arg"
func (s *S) MArg(arg int) {}

//- StringBuiltin=vname("string#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- StringBuiltin.code/rendered/callsite_signature "string"
//- ErrorBuiltin=vname("error#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- ErrorBuiltin.code/rendered/callsite_signature "error"

// - @H defines/binding H
// - H.code/rendered/callsite_signature "H(param)"
// - H.code/rendered/signature "func H(param func() (string, error)) error"
func H(param func() (string, error)) error { return nil }

// - @N defines/binding N
// - N.code/rendered/callsite_signature "N()"
// - N.code/rendered/signature "func N() (_ []*Rand, err error)"
func N() (_ []*rand.Rand, err error) {
	return nil, nil
}

// - @Set defines/binding Set
// - Set.code/rendered/signature "type Set[T comparable]"
type Set[T comparable] struct{}

// - @Insert defines/binding Insert
// - Insert.code/rendered/signature "func (s *Set[T]) Insert(t T)"
func (s *Set[T]) Insert(t T) {}

// - @G defines/binding G
// - G.code/rendered/qualified_name "rendered.G"
// - G.code/rendered/callsite_signature "G[T](t)"
// - G.code/rendered/signature "func G[T comparable](t T) *Set[T]"
func G[T comparable](t T) *Set[T] { return nil }

// - @#0Str defines/binding StrF
// - StrF.code/rendered/qualified_name "rendered.Str"
// - StrF.code/rendered/callsite_signature "Str[T](t)"
// - StrF.code/rendered/signature "func Str[T Stringer](t T) string"
func Str[T fmt.Stringer](t T) string { return t.String() }

// - @VA defines/binding VA
// - VA.code/rendered/callsite_signature "VA(args)"
// - VA.code/rendered/signature "func VA(args ...any)"
func VA(args ...any) {}

// - @I defines/binding I
// - I.code/rendered/signature "type I"
type I interface {
	// - @M defines/binding IM
	// - IM.code/rendered/signature "func (I) M(int) bool"
	M(int) bool
}
