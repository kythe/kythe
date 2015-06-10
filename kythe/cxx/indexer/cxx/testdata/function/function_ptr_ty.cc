// Checks that function pointer types are correctly recorded.
//- @A defines AliasA
//- AliasA aliases FnFTy
//- FnFTy.node/kind TApp
//- FnFTy param.0 vname("fn#builtin",_,_,_,_)
//- FnFTy param.1 vname("int#builtin",_,_,_,_)
//- FnFTy param.2 vname("float#builtin",_,_,_,_)
//- FnFTy param.3 vname("short#builtin",_,_,_,_)
using A = int (float, short);
//- @T defines AliasT
//- AliasT aliases PFnFTy
//- PFnFTy.node/kind TApp
//- PFnFTy param.0 vname("ptr#builtin",_,_,_,_)
//- PFnFTy param.1 FnFTy
typedef int (*T)(float, short);