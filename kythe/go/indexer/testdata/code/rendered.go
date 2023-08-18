package rendered

// - @F defines/binding F
// - F.code/rendered/qualified_name "rendered.F"
// - F.code/rendered/callsite_signature "F()"
func F() {}

// - @S defines/binding S
// - S.code/rendered/qualified_name "rendered.S"
type S struct{}

// - @M defines/binding M
// - M.code/rendered/qualified_name "rendered.S.M"
// - M.code/rendered/callsite_signature "(s) M()"
func (s *S) M() {}

// - @MArg defines/binding MArg
// - MArg.code/rendered/qualified_name "rendered.S.MArg"
// - MArg.code/rendered/callsite_signature "(s) MArg(arg)"
// - @arg defines/binding Arg
// - Arg.code/rendered/qualified_name "rendered.S.MArg.arg"
func (s *S) MArg(arg int) {}
