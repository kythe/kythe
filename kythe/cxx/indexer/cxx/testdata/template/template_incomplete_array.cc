// We don't fall over on incomplete arrays.
//- @T defines/binding TyvarT
template<typename T> struct A {
//- @R defines/binding FieldR
  T R[];
};
//- FieldR typed TAppIArrT
//- TAppDArrIntExpr.node/kind tapp
//- TAppDArrIntExpr param.0 vname("iarr#builtin",_,_,_,_)
//- TAppDArrIntExpr param.1 TyvarT
