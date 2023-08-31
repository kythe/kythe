package rendered

import "math/rand"

// - @F defines/binding F
// - F.code/rendered/qualified_name "rendered.F"
// - F.code/rendered/callsite_signature "F()"
func F() {}

// - @S defines/binding S
// - S.code/rendered/qualified_name "rendered.S"
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
