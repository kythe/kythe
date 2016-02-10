// We don't fall over on dependent-sized arrays.
//- @S defines/binding TyvarS
template<unsigned S> struct A {
//- @S ref TyvarS
//- @R defines/binding FieldR
  int R[S];
};
//- FieldR typed TAppDArrIntExpr
//- TAppDArrIntExpr.node/kind tapp
//- TAppDArrIntExpr param.0 vname("darr#builtin",_,_,_,_)
//- TAppDArrIntExpr param.1 vname("int#builtin",_,_,_,_)
//- TAppDArrIntExpr param.2 SomeExpr
