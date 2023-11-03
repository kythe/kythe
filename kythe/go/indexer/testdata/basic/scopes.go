package scopes

// - @Ident defines/binding Ident
// - Ident.node/kind variable
// - !{ Ident.subkind _ }
var Ident bool

// - @F defines/binding F
func F() {
	//- IdentRef=@Ident ref/writes Ident
	//- IdentRef childof F
	Ident = true

	//- AnonDef=@"func() {}" defines _Anon
	//- AnonDef childof F
	_ = func() {}
}
