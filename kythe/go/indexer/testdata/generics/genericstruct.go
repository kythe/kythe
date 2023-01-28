package genericstruct

func main() {
	//- @"Container[string]" ref ContainerApp
	//- @Container ref Container
	//- @string ref String
	c := &Container[string]{"element"}
	//- ContainerApp.node/kind "tapp"
	//- ContainerApp param.0 Container
	//- ContainerApp param.1 String

	//- @Element ref Element
	_ = c.Element

	//- @"Pair[string, int]" ref PairApp
	//- @Pair ref Pair
	//- @string ref String
	//- @int ref Int
	_ = &Pair[string, int]{"first", 2}
	//- PairApp.node/kind "tapp"
	//- PairApp param.0 Pair
	//- PairApp param.1 String
	//- PairApp param.2 Int
}

// - @Container defines/binding Container
// - Container.node/kind record
// - @T defines/binding TVar
// - TVar.node/kind tvar
// - Container tparam.0 TVar
type Container[T any] struct {
	//- @T ref TVar
	//- @Element defines/binding Element
	Element T

	//- Element.node/kind variable
	//- Element.subkind field
}

// - @T defines/binding TVar2
// - @U defines/binding UVar
// - !{@T defines/binding TVar}
// - !{@T ref TVar}
type Pair[T any, U any] struct {
	//- @T ref TVar2
	//- !{@T defines/binding TVar}
	//- !{@T ref TVar}
	First T
	//- @U ref UVar
	Second U
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
