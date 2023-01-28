// Package locals tests bindings in local scopes.
package locals

// - @foo defines/binding Foo
func foo() {
	//- @alpha defines/binding Alpha1
	//- Alpha1.node/kind variable
	//- Alpha1 childof Foo
	var alpha int

	// Short declaration form introduces only new names.
	// The others are references to their original definitions.
	//
	//- @bravo defines/binding Bravo
	//- Bravo.node/kind variable
	//- @alpha ref/writes Alpha1
	//- !{@alpha defines/binding Alpha1}
	alpha, bravo := 1, 2

	// Bindings in a local scope shadow their enclosing scope.
	//
	//- @alpha defines/binding Alpha2
	//- Alpha2.node/kind variable
	//- Alpha2 childof Foo
	for alpha := range []string{} {
		// Verify that the inner binding shadows the outer.
		//- @alpha ref Alpha2
		_ = alpha
	}

	// Don't choke on blanks in short assignment forms.
	//
	//- @y defines/binding Val
	//- Val.node/kind variable
	for _, y := range []string{} {
		//- @y ref Val
		print(y)
	}

	//- @#0alpha defines/binding Alpha3
	//- Alpha3.node/kind variable
	//-
	//- @#1alpha ref Alpha1
	//- @#2alpha ref Alpha3
	//- @bravo ref Bravo
	if alpha := alpha + 3; alpha < bravo {
		//- @bravo ref/writes Bravo
		//- @alpha ref Alpha3
		bravo = alpha
	}

	//- @alpha ref Alpha1
	_ = alpha

	//- @bravo ref Bravo
	_ = bravo
}
