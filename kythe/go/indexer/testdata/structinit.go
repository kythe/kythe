package structinit

// - @Inky defines/binding Inky
// - Inky.node/kind record
// - Inky.subkind struct
type Inky struct {
	//- @Pinky defines/binding Pinky
	//- Pinky.node/kind variable
	Pinky string

	//- @Blinky defines/binding Blinky
	//- Blinky.node/kind variable
	Blinky []byte

	//- @Sue defines/binding Sue
	//- Sue.node/kind variable
	Sue int
}

func msPacMan() {
	// Verify that named initializers ref/init their fields, and that the names
	// ref the fields.
	a := &Inky{
		//- @Pinky ref Pinky
		//- @"\"pink\"" ref/init Pinky
		Pinky: "pink",
		//- @Blinky ref Blinky
		//- @"[]byte{255, 0, 0}" ref/init Blinky
		Blinky: []byte{255, 0, 0},
		//- @Sue ref Sue
		//- @"0x84077e" ref/init Sue
		Sue: 0x84077e,
	}
	_ = a

	// Verify that unnamed initializers ref/init their fields.
	b := &Inky{
		//- @"a.Pinky" ref/init Pinky
		a.Pinky,

		//- @"[]byte{255, 0, 0}" ref/init Blinky
		[]byte{255, 0, 0},

		//- @"0x84077e" ref/init Sue
		0x84077e,
	}
	_ = b
}

func realNames() {
	//- @ghost defines/binding Ghost
	type ghost struct {
		//- @name defines/binding Name
		//- @nick defines/binding Nick
		name, nick string
	}

	// Verify that fields composite literals without explicit type names
	// correctly point back to the fields of their defining type.

	//- @ghost ref Ghost
	_ = []ghost{
		//- @"\"bashful\"" ref/init Name
		//- @"\"inky\"" ref/init Nick
		{"bashful", "inky"},
		//- @"\"speedy\"" ref/init Name
		//- @"\"pinky\"" ref/init Nick
		{"speedy", "pinky"},
		//- @"\"shadow\"" ref/init Name
		//- @"\"blinky\"" ref/init Nick
		{"shadow", "blinky"},

		//- @name ref Name
		//- @"\"pokey\"" ref/init Name
		//- @nick ref Nick
		//- @"\"clyde\"" ref/init Nick
		{name: "pokey", nick: "clyde"},

		// Order and missing fields should not cause problems.
		//- @nick ref Nick
		//- @"\"sue\"" ref/init Nick
		//- @name ref Name
		//- @"\"Susannah\"" ref/init Name
		{nick: "sue", name: "Susannah"},

		//- @nick ref Nick
		//- @"\"kyle\"" ref/init Nick
		{nick: "kyle"},
	}
}
