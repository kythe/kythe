// Package impl tests implementation relationships.
package impl

// - @Busy defines/binding BusyInterface
type Busy interface {
	//- @Do defines/binding DoMethod
	//- DoMethod childof BusyInterface
	Do()

	//- @Be defines/binding BeMethod
	//- BeMethod childof BusyInterface
	Be()
}

// - @Phil defines/binding Phil
// - Phil satisfies Busy
type Phil int

func (Phil) Do() {}
func (Phil) Be() {}

// - @Bad1 defines/binding DoOnly
// - !{ DoOnly satisfies Busy}
type Bad1 bool

// - @Bad2 defines/binding BeOnly
// - !{ BeOnly satisfies Busy}
type Bad2 float64
