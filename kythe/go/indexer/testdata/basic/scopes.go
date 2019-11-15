package scopes

//- @Ident defines/binding Ident
var Ident bool

//- @F defines/binding F
func F() {
	//- IdentRef=@Ident ref Ident
	//- IdentRef childof F
	Ident = true
}
