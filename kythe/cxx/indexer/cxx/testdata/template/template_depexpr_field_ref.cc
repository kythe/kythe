// Checks that we don't fall over on fields that depend on expressions.
//- @T defines/binding TyvarT
template <typename T> struct S {
  T t;
  //- @f ref DepF
  //- DepF.node/kind lookup
  //- DepF.text f
  //- !{DepF param.0 Anything}
  //- @thing ref DepThing
  //- DepThing.node/kind lookup
  //- DepThing param.0 TyvarT
  int i = (t.thing(3) + 4).f;
};
