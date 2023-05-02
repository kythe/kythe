package meta

func Foobar() {}

//   ^     ^ offset 25
//   \ offset 19

func Barfoo() {}

//   ^     ^ offset 84
//   \ offset 78

func SetFoo() {}

//   ^     ^ offset 143
//   \ offset 137

// Note: The locations in this file are connected to the offsets defined in the
// associated meta file. If you move anything above this comment without
// updating the metadata, the test may break.

//- FA.node/kind anchor
//- FA.loc/start 19
//- FA.loc/end   25
//- FA defines/binding Foobar
//- Foobar.node/kind function
//- _Alt=vname(gsig, gcorp, groot, gpath, glang) generates Foobar
//- FB.node/kind anchor
//- FB.loc/start 78
//- FB.loc/end   84
//- FB defines/binding Barfoo
//- Barfoo.node/kind function
//- _AltB=vname(gsig2, gcorp, groot, gpath, glang) generates Barfoo
//- vname("", gcorp, groot, gpath, "") generates vname("", kythe, _, "go/indexer/metadata_test/meta.go", "")

//- SA.node/kind anchor
//- SA.loc/start 137
//- SA.loc/end 143
//- SA defines/binding SetFoo
//- SetFoo.semantic/generated set
