package genericfunc

//- @Map defines/binding Func
//- Func.node/kind function
//- @#0T defines/binding TVar
//- TVar.node/kind tvar
//- @#0U defines/binding UVar
//- UVar.node/kind tvar
//- Func tparam.0 TVar
//- Func tparam.1 UVar
func Map[T any, U any](l []T, f func(T) U) []U {
	//- @U ref UVar
	res := make([]U, len(l))
	for i := 0; i < len(l); i++ {
		//- @T ref TVar
		var t T = l[i]
		res[i] = f(t)
	}
	return res
}

// kythe/go/indexer/genericfunc_test.Map.T
//- TVar code TVarCode
//- TVarCode.kind "BOX"
//- TVarCode child.0 C
//- TVarCode child.1 I
//- C.kind "CONTEXT"
//- C.post_child_text "."
//- C.add_final_list_token "true"
//- C child.0 Pkg
//- C child.1 FuncCode
//- Pkg.kind "IDENTIFIER"
//- Pkg.pre_text "kythe/go/indexer/genericfunc_test"
//- FuncCode.kind "IDENTIFIER"
//- FuncCode.pre_text "Map"
//- I.kind "IDENTIFIER"
//- I.pre_text "T"
