package dependent

import (
	"fmt"

	dep "kythe/go/indexer/dep.v2"
	types "kythe/go/indexer/types_test"
)

// Repeat the test from types.go to ensure the references work across packages.
func f(
	i types.StringerI,
	s fmt.Stringer,
	e types.StringerE,
	e2 types.StringerE2,
	a types.StringerA,
) {
	//- @String ref String
	i.String()
	//- @String ref String
	s.String()
	//- @String ref String
	e.String()
	//- @String ref String
	e2.String()
	//- @String ref String
	a.String()
}

func g() {
	//- @F ref F
	dep.F()
}
