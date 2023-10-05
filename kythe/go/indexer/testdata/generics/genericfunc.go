package genericfunc

func main() {
	//- @Map ref Func
	Map([]string{}, func(s string) string { return s })
}

// - @Map defines/binding Func
// - Func.node/kind function
// - @#0T defines/binding TVar
// - TVar.node/kind tvar
// - @#0U defines/binding UVar
// - UVar.node/kind tvar
// - Func tparam.0 TVar
// - Func tparam.1 UVar
func Map[T any, U comparable](l []T, f func(T) U) []U {
	//- @U ref UVar
	res := make([]U, len(l))
	for i := 0; i < len(l); i++ {
		//- @T ref TVar
		var t T = l[i]
		res[i] = f(t)
	}
	return res
}

//- Func code FuncCode
//- FuncCode.kind "BOX"
//- FuncCode child.2 FuncParams
//- FuncParams.kind "PARAMETER"
//- FuncParams.pre_text "["
//- FuncParams.post_text "]"
//- FuncParams.post_child_text ", "
//- FuncParams child.0 TParam
//- TParam child.0 TParamIdent
//- TParamIdent.pre_text "T"
//- TParam child.1 TParamConstraint
//- TParamConstraint.pre_text "any"
//- FuncParams child.1 UParam
//- UParam child.0 UParamIdent
//- UParamIdent.pre_text "U"
//- UParam child.1 UParamConstraint
//- UParamConstraint.pre_text "comparable"

// kythe/go/indexer/genericfunc_test.Map.T
//- TVar code TVarCode
//- TVarCode.kind "BOX"
//- TVarCode child.0 C
//- TVarCode child.1 I
//- C.kind "CONTEXT"
//- C.post_child_text "."
//- C.add_final_list_token "true"
//- C child.0 Pkg
//- C child.1 FuncCtxCode
//- Pkg.kind "IDENTIFIER"
//- Pkg.pre_text "kythe/go/indexer/genericfunc_test"
//- FuncCtxCode.kind "IDENTIFIER"
//- FuncCtxCode.pre_text "Map"
//- I.kind "IDENTIFIER"
//- I.pre_text "T"
