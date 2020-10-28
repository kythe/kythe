// Package pkginit tests callgraph relationships for calls occurring in the
// package-level initializer.
package pkginit

import "fmt"

// Ensure that callsites at the package level are blamed on the implicit
// package initializer, and that said initializer is given a sensible
// definition anchor per http://www.kythe.io/docs/schema/callgraph.html

//- A=@"fmt.Sprint(27)" ref/call _FmtSprint
//- A childof PkgInit=vname("package.<init>@107", "test", _, "pkginit", "go")
//- PkgInit.node/kind function
//-
//- InitDef=vname(_, "test", "", "pkginit/packageinit.go", "go")
//-     defines PkgInit
//- InitDef.node/kind anchor
//- InitDef.loc/start 0
//- InitDef.loc/end 0
var p = fmt.Sprint(27)
