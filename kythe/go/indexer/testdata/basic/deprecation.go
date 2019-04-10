// Package deprecate tests deprecation.

//- @+3dep defines/binding Pkg

// Deprecated: dep should not be used
package dep

//- Pkg.node/kind package
//- Pkg.deprecated "dep should not be used"

//- @+6topLevel defines/binding TopLevel
//- TopLevel.node/kind variable
//- TopLevel.deprecated "topLevel has insufficient precision"
//- TopLevel childof Pkg

// Deprecated: topLevel has insufficient precision
var topLevel int

//- @+4outer defines/binding Outer
//- Outer.deprecated "outer has been replaced by inner"

// Deprecated: outer has been replaced by inner
func outer() {
	//- @+6stabby defines/binding V
	//- V.node/kind variable
	//- V childof Outer
	//- V.deprecated "stabby is too sharp"

	// Deprecated: stabby is too sharp
	var stabby bool

	_ = stabby // suppress unused variable error
}

//- @+6multilineDep defines/binding Func
//- Func.deprecated "more than one"

// Deprecated: more than one
// line for deprecation
// message
func multilineDep() {
}

//- @+6magic defines/binding Const
//- Const.node/kind constant
//- Const.deprecated "use technology instead"
//- Const childof Pkg

// Deprecated: use technology instead
const magic = "beans"
