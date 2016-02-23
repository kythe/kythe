// Checks indexing dependent constructors.
//- @T defines/binding TyvarT
template <typename T>
struct S : T::Q {
  //- @"Q()" ref/call LookupTQCtorC
  //- LookupTQCtor callableas LookupTQCtorC
  //- LookupTQCtor.node/kind lookup
  //- LookupTQCtor.text "#ctor"
  //- LookupTQCtor param.0 LookupTQ
  //- LookupTQ.text "Q"
  //- LookupTQ param.0 TyvarT
  S() : T::Q() { }
};
