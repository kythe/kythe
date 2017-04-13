// Package override tests that override edges are correcty generated between
// methods of concrete types and the interfaces they satisfy.
package override

//- @Thinger defines/binding Thinger
//- Thinger.node/kind interface
type Thinger interface {
	//- @Thing defines/binding AbstractThing
	//- Thing.node/kind function
	//- Thing childof Thinger
	Thing()
}

//- @Stuffer defines/binding Stuffer
//- Stuffer.node/kind interface
//- Stuffer extends Thinger
type Stuffer interface {
	Thinger

	//- @Stuff defines/binding AbstractStuff
	//- Stuff.node/kind function
	//- Stuff childof Stuffer
	Stuff()
}

//- @foo defines/binding Foo
//- Foo.node/kind record
type foo struct{}

//- @Thing defines/binding ConcreteThing
//- ConcreteThing.node/kind function
//- ConcreteThing childof Foo
//- ConcreteThing overrides AbstractThing
func (foo) Thing() {}

//- @Stuff defines/binding ConcreteStuff
//- ConcreteStuff.node/kind function
//- ConcreteStuff childof Foo
//- ConcreteStuff overrides AbstractStuff
func (foo) Stuff() {}
