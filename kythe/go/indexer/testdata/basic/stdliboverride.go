// Package stdliboverride tests that corpus labels are assigned correctly when
// the --override_stdlib_corpus and --use_compilation_corpus_as_default flags
// are enabled.
package stdliboverride

import (
    //- @"\"bytes\"" ref/imports BYTES=vname("package","STDLIB_OVERRIDE",_,"bytes","go")
	"bytes"
)

//- @myFunc=vname(_,"kythe",_,_,"go") defines/binding _MY_FUNC
func myFunc() {
    //- @bytes ref BYTES
    //- @NewBuffer ref vname(_,"STDLIB_OVERRIDE",_,_,"go")
	bytes.NewBuffer(nil)
}
