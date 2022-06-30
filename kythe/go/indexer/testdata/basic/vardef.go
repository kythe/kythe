// Package def tests variable and constant definitions.
// - @def defines/binding Pkg
package def

//- Pkg.node/kind package

// - @topLevel defines/binding TopLevel
// - TopLevel.node/kind variable
// - TopLevel childof Pkg
var topLevel int

// - @outer defines/binding Outer
func outer() {
	//- @stabby defines/binding V
	//- V.node/kind variable
	//- V childof Outer
	var stabby bool

	_ = stabby // suppress unused variable error
}

// - @magic defines/binding Const
// - Const.node/kind constant
// - Const childof Pkg
const magic = "beans"
