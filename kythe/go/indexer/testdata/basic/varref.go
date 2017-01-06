// Package ref tests variable references.
package ref

var (
	T int

	//- @T ref TVar
	U = T + 1
)
