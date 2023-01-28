package scopes

// - @Ident defines/binding Ident
var Ident bool

// - @F defines/binding F
func F() {
	//- IdentRef=@Ident ref/writes Ident
	//- IdentRef childof F
	Ident = true
}
