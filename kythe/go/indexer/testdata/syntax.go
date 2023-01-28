// Package syntax contains tricky syntactic forms that the indexer needs to
// handle correctly. This test "passes" if the indexer doesn't crash.
package syntax

type oldStruct struct {
	n int
}

// A struct type that is an alias doesn't add new fields.
type newStruct oldStruct

type oldInterface interface {
	foo()
}

// An interface type that is an alias doesn't add new methods.
type newInterface oldInterface
