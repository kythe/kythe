// Package tspec tests properties of type declarations.
package tspec

//- @Int defines/binding Int
//- @"Int int" defines Int
type Int int

//- @Ptr defines/binding Ptr
//- @"Ptr *bool" defines Ptr
//- Ptr.node/kind tapp
type Ptr *bool

//- @Struct defines/binding Struct
//- Struct.node/kind record
//- Struct.subkind struct
type Struct struct {
	//- @Alpha defines/binding Alpha
	//- Alpha.node/kind variable
	//- Alpha childof Struct
	Alpha string

	//- @Bravo defines/binding Bravo
	//- Bravo.node/kind variable
	//- Bravo childof Struct
	Bravo int
}

//- @Thinger defines/binding Thinger
//- Thinger.node/kind interface
type Thinger interface {
	//- @Thing defines/binding Thing
	//- Thing.node/kind function
	//- Thing childof Thinger
	Thing()
}

//- @Extender defines/binding Extender
//- Extender.node/kind interface
//- Extender extends Thinger
type Extender interface {
	Thinger

	//- @Extend defines/binding Extend
	//- Extend.node/kind function
	//- Extend childof Extender
	Extend()
}
