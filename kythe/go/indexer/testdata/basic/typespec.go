// Package tspec tests properties of type declarations.
package tspec

import "fmt"

// - @Int defines/binding Int
// - @"Int int" defines Int
// - Int.node/kind record
// - Int.subkind type
type Int int

// - @Ptr defines/binding Ptr
// - @"Ptr *bool" defines Ptr
// - Ptr.node/kind record
// - Ptr.subkind type
type Ptr *bool

type under struct{ z int }

// - @Over defines/binding Over
// - Over.node/kind record
// - Over.subkind struct
type Over under

// - @UPtr defines/binding UPtr
// - UPtr.node/kind record
// - UPtr.subkind type
type UPtr *under

// - @Struct defines/binding Struct
// - Struct.node/kind record
// - Struct.subkind struct
type Struct struct {
	//- @Alpha defines/binding Alpha
	//- Alpha.node/kind variable
	//- Alpha.subkind field
	//- Alpha childof Struct
	Alpha string

	//- @Bravo defines/binding Bravo
	//- Bravo.node/kind variable
	//- Bravo.subkind field
	//- Bravo childof Struct
	Bravo int
}

// - @Embed defines/binding Embed
// - Embed.node/kind record
// - Embed.subkind struct
type Embed struct {
	// An embedded type from this package.
	//
	//- @"Struct" defines/binding EmbedStruct
	//- EmbedStruct.node/kind variable
	//- EmbedStruct.subkind field
	//- @"Struct" ref Struct
	Struct

	// An embedded pointer type.
	//
	//- @"float64" defines/binding EmbedFloat
	//- EmbedFloat.node/kind variable
	//- EmbedFloat.subkind field
	*float64

	// A type from another package.
	//
	//- @"Stringer" defines/binding FmtStringer
	//- FmtStringer.node/kind variable
	//- FmtStringer.subkind field
	fmt.Stringer

	// A regular field mixed with the above.
	//
	//- @Velocipede defines/binding Velo
	//- Velo.node/kind variable
	//- Velo.subkind field
	Velocipede struct{}
}

// - @Thinger defines/binding Thinger
// - Thinger.node/kind interface
type Thinger interface {
	//- @Thing defines/binding Thing
	//- Thing.node/kind function
	//- Thing childof Thinger
	//- @param defines/binding Param
	//- Param childof Thing
	Thing(param int)
}

// - @Extender defines/binding Extender
// - Extender.node/kind interface
// - Extender extends Thinger
type Extender interface {
	Thinger

	//- @Extend defines/binding Extend
	//- Extend.node/kind function
	//- Extend childof Extender
	Extend()
}
