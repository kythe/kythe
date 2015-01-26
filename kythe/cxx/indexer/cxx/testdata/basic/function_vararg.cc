// Checks how we record functions with varargs.
//- @F defines VarargFn
//- VarargFn typed TAppVarargFn
//- TAppVarargFn.node/kind tapp
//- TAppVarargFn param.0 vname("fnvararg#builtin",_,_,_,_)
//- TAppVarargFn param.1 vname("void#builtin",_,_,_,_)
//- TAppVarargFn param.2 vname("int#builtin",_,_,_,_)
void F(int I, ...) {
  F(I, 1, 2, 3);
}
