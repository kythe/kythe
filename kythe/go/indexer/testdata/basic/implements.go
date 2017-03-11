// Package impl tests implementation relationships.
//
// TODO(fromberger): Update the edge labels here when we have decided what to
// do about T224.
package impl

//- @Busy defines/binding BusyInterface
type Busy interface {
	//- @Do defines/binding DoMethod
	//- DoMethod childof BusyInterface
	Do()

	//- @Be defines/binding BeMethod
	//- BeMethod childof BusyInterface
	Be()
}

//- @Phil defines/binding Phil
//- Phil extends Busy
type Phil int

func (Phil) Do() {}
func (Phil) Be() {}

//- @Bad1 defines/binding DoOnly
//- !{ DoOnly extends Busy}
type Bad1 bool

//- @Bad2 defines/binding BeOnly
//- !{ BeOnly extends Busy}
type Bad2 float64
