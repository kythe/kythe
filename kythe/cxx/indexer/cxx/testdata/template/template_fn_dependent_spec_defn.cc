// Checks indexing refs and defs of dependent function specializations.
//- @f defines/binding AbsF
template <typename S> long f(S s) { return 0; }
template <typename T> struct S {
  friend
  //- @f defines/binding DepSpecFT
  //- DepSpecFT specializes/speculative TAppAbsFT
  //- TAppAbsFT param.0 AbsF
  //- TAppAbsFT param.1 BuiltinInt
  //- TAppAbsFT param.2 BuiltinShort
  //- @int ref BuiltinInt
  //- @short ref BuiltinShort
  long f<int, short>(T t) { return 1; }
};
