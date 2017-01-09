// Package ref tests variable references.
package ref

var (
	//- @T defines/binding TVar
	T int

	//- @T ref TVar
	//- TVar.node/kind variable
	U = T + 1
)
