package genericinterface

//- @Interface defines/binding Interface
//- Interface.node/kind interface
//- @T defines/binding TVar
//- TVar.node/kind tvar
//- Interface tparam.0 TVar
type Interface[T any] interface {
	//- @Accept defines/binding Accept
	//- @T ref TVar
	Accept(T)
}

//- @Thing defines/binding Thing
//- Thing satisfies Interface
type Thing struct{ S string }

//- @Accept defines/binding ThingAccept
//- ThingAccept overrides Accept
func (t *Thing) Accept(s string) { t.S = s }

//- @Interface ref Interface
var _ Interface[string] = &Thing{"hello"}

//- @Container defines/binding Container
//- Container satisfies Interface
type Container[T any] struct{ Element T }

//- @Accept defines/binding ContainerAccept
//- ContainerAccept overrides Accept
func (c *Container[T]) Accept(t T) { c.Element = t }

// kythe/go/indexer/genericinterface_test.Interface.T
//- TVar code TVarCode
//- TVarCode.kind "BOX"
//- TVarCode child.0 C
//- TVarCode child.1 I
//- C.kind "CONTEXT"
//- C.post_child_text "."
//- C.add_final_list_token "true"
//- C child.0 Pkg
//- C child.1 Struct
//- Pkg.kind "IDENTIFIER"
//- Pkg.pre_text "kythe/go/indexer/genericinterface_test"
//- Struct.kind "IDENTIFIER"
//- Struct.pre_text "Interface"
//- I.kind "IDENTIFIER"
//- I.pre_text "T"
