package genericmethod

func main() {
	c := &Container[string]{"element"}
	//- @Get ref Get
	c.Get()
	//- @Put ref Put
	c.Put("yup")

	//- @getter defines/binding Getter
	//- @Get ref Get
	getter := c.Get
	//- @getter ref Getter
	getter()
}

// - @Container defines/binding Container
// - @T defines/binding TVar
type Container[T any] struct {
	//- @T ref TVar
	Element T
}

// Methods introduce unique tvars
// TODO(schroederc): relate these to the struct tvar
// - @Get defines/binding Get
// - @#0T defines/binding GetTVar
// - GetTVar.node/kind tvar
// - @#1T ref GetTVar
// - !{@#0T ref TVar}
// - !{@#1T ref TVar}
// - @"Container[T]" ref TApp
// - TApp.node/kind tapp
// - TApp param.0 Container
// - TApp param.1 GetTVar
func (c *Container[T]) Get() T {
	//- @T ref GetTVar
	//- !{@T ref TVar}
	var res T = c.Element
	return res
}

//- Get code GetCode
//- GetCode.kind "BOX"
//- GetCode child.1 RecvCode
//- RecvCode child.0 LookupCode
//- LookupCode.kind "LOOKUP_BY_PARAM"

// And can technically be renamed
// - @Put defines/binding Put
// - @#0A defines/binding PutTVar
// - PutTVar.node/kind tvar
// - @#1A ref PutTVar
// - @#2A ref PutTVar
// - !{@#0A ref GetTVar}
// - !{@#1A ref GetTVar}
// - !{@#2A ref GetTVar}
func (c *Container[A]) Put(t A) A {
	//- @A ref PutTVar
	//- !{@A ref GetTVar}
	var temp A = c.Element
	c.Element = temp
	return temp
}

// - @Interface defines/binding Interface
type Interface interface {
	//- @Get defines/binding GetI
	Get() string
	//- @Put defines/binding PutI
	Put(string) string

	//- Container satisfies Interface
	//- Get overrides GetI
	//- Put overrides PutI
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
