package genericmethod

//- @T defines/binding TVar
type Container[T any] struct {
	//- @T ref TVar
	Element T
}

// Methods introduce unique tvars
// TODO(schroederc): relate these to the struct tvar
//- @#0T defines/binding GetTVar
//- GetTVar.node/kind tvar
//- @#1T ref GetTVar
//- !{@#0T ref TVar}
//- !{@#1T ref TVar}
func (c *Container[T]) Get() T {
	//- @T ref GetTVar
	//- !{@T ref TVar}
	var res T = c.Element
	return res
}

// And can technically be renamed
//- @#0A defines/binding PutTVar
//- PutTVar.node/kind tvar
//- @#1A ref PutTVar
//- @#2A ref PutTVar
//- !{@#0A ref GetTVar}
//- !{@#1A ref GetTVar}
//- !{@#2A ref GetTVar}
func (c *Container[A]) Put(t A) A {
	//- @A ref PutTVar
	//- !{@A ref GetTVar}
	var temp A = c.Element
	c.Element = temp
	return temp
}

// kythe/go/indexer/genericmethod_test.Container.Get.T
//- GetTVar code TVarCode
//- TVarCode.kind "BOX"
//- TVarCode child.0 C
//- TVarCode child.1 I
//- C.kind "CONTEXT"
//- C.post_child_text "."
//- C.add_final_list_token "true"
//- C child.0 Pkg
//- C child.1 Struct
//- C child.2 Method
//- Pkg.kind "IDENTIFIER"
//- Pkg.pre_text "kythe/go/indexer/genericmethod_test"
//- Struct.kind "IDENTIFIER"
//- Struct.pre_text "Container"
//- Method.kind "IDENTIFIER"
//- Method.pre_text "Get"
//- I.kind "IDENTIFIER"
//- I.pre_text "T"
