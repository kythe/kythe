// Package override tests that override edges are correcty generated between
// methods of concrete types and the interfaces they satisfy.
package override

import _ "fmt"

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
//- Foo satisfies Thinger
//- Foo satisfies Stuffer
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

//- @bar defines/binding Bar
//- Bar.node/kind record
//- Bar satisfies Thinger
//- Bar satisfies vname("type Stringer",_,_,"fmt","go")
//- !{ Bar satisfies Stuffer }
type bar struct{}

//- @Thing defines/binding OtherConcreteThing
//- OtherConcreteThing.node/kind function
//- OtherConcreteThing childof Bar
//- OtherConcreteThing overrides AbstractThing
func (*bar) Thing() {}

//- @String defines/binding String
//- String.node/kind function
//- String childof Bar
//- String overrides vname("method Stringer.String",_,_,"fmt","go")
func (*bar) String() string { return "" }
