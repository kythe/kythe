// Package override tests that override edges are correcty generated between
// methods of concrete types and the interfaces they satisfy.
package override

import (
	_ "fmt"
	"strings"
)

//- @Thinger defines/binding Thinger
//- Thinger.node/kind interface
type Thinger interface {
	//- @Thing defines/binding AbstractThing
	//- Thing.node/kind function
	//- Thing childof Thinger
	//- AbstractThing typed AbstractThingType
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
	//- AbstractStuff typed AbstractStuffType
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
//- ConcreteThing typed ConcreteThingType
//- ConcreteThingType satisfies AbstractThingType
//- !{ ConcreteThing overrides FoilThing }
func (foo) Thing() {}

//- @Stuff defines/binding ConcreteStuff
//- ConcreteStuff.node/kind function
//- ConcreteStuff childof Foo
//- ConcreteStuff overrides AbstractStuff
//- ConcreteStuff typed ConcreteStuffType
//- ConcreteStuffType satisfies AbstractStuffType
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
//- !{ OtherConcreteThing overrides FoilThing }
func (*bar) Thing() {}

//- @String defines/binding String
//- String.node/kind function
//- String childof Bar
//- String overrides vname("method Stringer.String",_,_,"fmt","go")
func (*bar) String() string { return "" }

// Foil has a method with the same name as Thinger, but is not a compatible
// type signature. We use this to verify that we don't try to emit override
// edges unless the assignability check passes.
//
//- @Foil defines/binding Foil
//- Foil.node/kind interface
type Foil interface {
	//- @Thing defines/binding FoilThing
	//- FoilThing.node/kind function
	//- FoilThing childof Foil
	Thing(bool)
}

//- @Grower defines/binding Grower
//- Grower.node/kind interface
//- StringBuilder satisfies Grower
type Grower interface {
	//- @Grow defines/binding Grow
	//- Grow.node/kind function
	Grow(int)

	//- StringBuilderGrow overrides Grow
}

//- @Builder ref StringBuilder
var _ Grower = &strings.Builder{}

//- @Grow ref StringBuilderGrow
func init() { (&strings.Builder{}).Grow(10) }
