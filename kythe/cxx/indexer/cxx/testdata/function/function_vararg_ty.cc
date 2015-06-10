// Checks how we record the types of functions with varargs.
//- @F defines AliasF
using F = void(int, ...);
//- AliasF aliases TApp
//- TApp.node/kind tapp
//- TApp param.0 vname("fnvararg#builtin",_,_,_,_)
//- TApp param.1 vname("void#builtin",_,_,_,_)
//- TApp param.2 vname("int#builtin",_,_,_,_)
