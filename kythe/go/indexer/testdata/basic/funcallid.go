// Package fun tests basic function call references.
// - @fun defines/binding Pkg
package fun

//- @"\"os/exec\"" ref/imports OSExec
import "os/exec"

//- Pkg.node/kind package
//- Init childof Pkg
//- Init.node/kind function

// - @F defines/binding Fun = vname("func F", "test", _, "fun", "go")
func F() int { return 0 }

type T struct{}

// - @M defines/binding Meth=vname("method T.M", "test", _, "fun", "go")
func (p *T) M() {}

// - @F ref Fun
// - TCall=@F ref/call Fun
// - TCall childof Init
var _ = F()

// - @init defines/binding InitFunc = vname("func init#1", "test", _, "fun", "go")
func init() {
	//- @F ref Fun
	//- FCall=@F ref/call Fun
	//- FCall childof InitFunc
	F()

	var t T

	//- @M ref Meth
	//- MCall=@M ref/call Meth
	//- MCall childof InitFunc
	t.M()
}

func imported() {
	//- @cmd defines/binding Cmd
	//- @exec ref OSExec
	//- @Command ref _ExecCommand
	cmd := exec.Command("pwd")

	//- @cmd ref Cmd
	//- @Run ref CmdRun=vname("method Cmd.Run","golang.org","","os/exec","go")
	//- @Run ref/call CmdRun
	cmd.Run()
}
