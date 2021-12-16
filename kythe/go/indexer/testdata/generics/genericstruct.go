package genericstruct

//- @Container defines/binding Container
//- Container.node/kind record
//- @T defines/binding TVar
//- TVar.node/kind tvar
//- Container tparam.0 TVar
type Container[T any] struct {
	//- @T ref TVar
	Element T
}

//- @T defines/binding TVar2
//- !{@T defines/binding TVar}
//- !{@T ref TVar}
type Container2[T any] struct {
	//- @T ref TVar2
	//- !{@T defines/binding TVar}
	//- !{@T ref TVar}
	Element T
}

// kythe/go/indexer/genericstruct_test.Container.T
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
//- Pkg.pre_text "kythe/go/indexer/genericstruct_test"
//- Struct.kind "IDENTIFIER"
//- Struct.pre_text "Container"
//- I.kind "IDENTIFIER"
//- I.pre_text "T"
