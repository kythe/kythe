// Package sref tests basic struct field and method references, including
// references across package boundaries.
package sref

import (
	//- @"\"os\"" ref/imports OS=vname("package","golang.org","","os",_)
	"os"
	//- @"\"os/exec\"" ref/imports Exec=vname("package","golang.org","","os/exec",_)
	"os/exec"
)

// Construct a struct value from another package.
//
// - @cmd defines/binding Cmd
// - Cmd.node/kind variable
// - @exec ref Exec
// - @"exec.Command(\"blah\")" ref/call ExecCommand
// - @Command ref ExecCommand
var cmd = exec.Command("blah")

// Verify that references to the struct's fields work.
//
// - @ofp defines/binding Out
// - Out.node/kind variable
// - @cmd ref Cmd
// - StdoutRef=@Stdout ref CmdStdout
// -   = vname("field Cmd.Stdout","golang.org","","os/exec","go")
// - ! {StdoutRef childof _}
var ofp = cmd.Stdout

// - @init defines/binding Init
// - Init.node/kind function
func init() {
	//- @cmd ref Cmd
	//- @Stdout ref CmdStdout
	//- @os ref OS
	//- @Stderr ref _OSStderr
	//-   = vname("var Stderr","golang.org","","os","go")
	cmd.Stdout = os.Stderr

	//- RunRef=@Run ref CmdRun
	//-   = vname("method Cmd.Run","golang.org","","os/exec","go")
	//- RunCall=@"cmd.Run()" ref/call CmdRun
	//- RunCall childof Init
	//- !{RunRef childof _}
	cmd.Run()
}
