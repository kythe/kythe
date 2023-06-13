// Checks indexing dependent constructors.
//- @T defines/binding TyvarT
template <typename T>
struct S : T::Q {
  //- @"T::Q()" ref/call LookupTQCtor
  //- LookupTQCtor.node/kind lookup
  //- LookupTQCtor.text "#ctor"
  //- LookupTQCtor param.0 LookupTQ
  //- LookupTQ.text "Q"
  //- LookupTQ param.0 TyvarT
  S() : T::Q() { }
};
