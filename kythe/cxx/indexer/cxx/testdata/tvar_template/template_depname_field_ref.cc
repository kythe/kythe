// Checks that we can refer to dependent fields.
//- @T defines/binding TyvarT
template <typename T> struct Sz {
  T t;
  //- @f ref DepF
  //- DepF.node/kind lookup
  //- DepF.text f
  //- DepF param.0 TyvarT
  int i = t.f;
};
