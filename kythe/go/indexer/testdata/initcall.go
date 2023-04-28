package initcall

import "fmt"

// Verify that duplicate anchors are not generated for function calls occurring
// in the initializer of a struct field.

// - @silly defines/binding Silly
type silly struct {
	//- @Q defines/binding SField
	//- SField childof Silly
	Q string
}

var s = silly{
	//- @Q ref/writes SField
	//- Call=@"fmt.Sprint(\"silly\")" ref/init SField
	//- Call ref/call Sprint=vname("func Sprint","golang.org",_,"fmt","go")
	//- @Sprint ref Sprint
	Q: fmt.Sprint("silly"),
}
