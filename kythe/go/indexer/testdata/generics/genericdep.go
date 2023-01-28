package genericdep

import (
	genericinterface "kythe/go/indexer/genericinterface_test"
)

func main() {
	c := &genericinterface.Container[string]{"element"}
	//- @Accept ref ContainerAccept
	c.Accept("yup")
	//- @Element ref/writes Element
	c.Element = ""
}

// - @Number defines/binding Number
// - Number satisfies Interface
type Number struct{ I int }

// - @Accept defines/binding NumberAccept
// - NumberAccept overrides Accept
func (n *Number) Accept(i int) { n.I = i }

// - @Interface ref Interface
var _ genericinterface.Interface[int] = &Number{42}
