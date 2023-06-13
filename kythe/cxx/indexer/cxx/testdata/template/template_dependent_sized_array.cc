// We don't fall over on dependent-sized arrays.
//- @S defines/binding TyvarS
//- @T defines/binding TyvarT
template<unsigned S, class... T> struct A {
//- @S ref TyvarS
//- @R defines/binding FieldR
  int R[S];

  void fn() {
    //- @Q defines/binding VarQ
    //- @T ref TyvarT
    int Q[] = {sizeof(T)...};
  }

  //- @Z defines/binding FieldZ
  //- @T ref TyvarT
  int Z[sizeof...(T)];
};
//- FieldR typed TAppDArrIntExpr
//- TAppDArrIntExpr.node/kind tapp
//- TAppDArrIntExpr param.0 vname("darr#builtin",_,_,_,_)
//- TAppDArrIntExpr param.1 vname("int#builtin",_,_,_,_)
//- TAppDArrIntExpr param.2 SomeExpr
//- VarQ typed QType
//- QType.node/kind tapp
//- QType param.0 vname("darr#builtin",_,_,_,_)
//- QType param.1 vname("int#builtin",_,_,_,_)
//- FieldZ typed ZType
//- ZType.node/kind tapp
//- ZType param.0 vname("darr#builtin",_,_,_,_)
//- ZType param.1 vname("int#builtin",_,_,_,_)
