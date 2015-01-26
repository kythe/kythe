// Checks that function types are correctly recorded.
//- @F defines FnF
//- FnF typed FnFTy
//- FnFTy.node/kind TApp
//- FnFTy param.0 vname("fn#builtin",_,_,_,_)
//- FnFTy param.1 vname("int#builtin",_,_,_,_)
//- FnFTy param.2 vname("float#builtin",_,_,_,_)
//- FnFTy param.3 vname("short#builtin",_,_,_,_)
int F(float A, short);
