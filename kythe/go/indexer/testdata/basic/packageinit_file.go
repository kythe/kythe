// Package pkginit tests callgraph relationships for calls occurring at the
// top-level file.
package pkginit

import "fmt"

// Ensure that callsites at the package level are blamed on the file.

//- A=@"fmt.Sprint(27)" ref/call _FmtSprint
//- A childof File
//- File.node/kind file
var p = fmt.Sprint(27)
