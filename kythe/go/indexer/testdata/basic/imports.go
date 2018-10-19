// Package imports tests references at import sites, and references to imported
// packages in the code.
package imports

import (
	//- @"\"fmt\"" ref/imports
	//-   Fmt=vname("package", "golang.org", _, "fmt", "go")
	"fmt"
	//- @"\"strconv\"" ref/imports
	//-   Strconv=vname("package", "golang.org", _, "strconv", "go")
	"strconv"

	//- @"\"net/http\"" ref/imports
	//-   _NetHTTP=vname("package", "golang.org", _, "net/http", "go")
	_ "net/http"
)

//- @fmt ref Fmt
var _ = fmt.Sprint
//- @strconv ref Strconv
var _ = strconv.Atoi
