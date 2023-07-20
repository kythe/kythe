// Checks indexing dependent constructors.
template <typename T>
struct M {
  M() { }
  M(int) { }
};

template <typename T>
struct S {
  S(M<T>) { }
};

template <typename T>
//- @L defines/binding AbsL
struct L : S<T> {
  //- StCall=@"S<T>(M<T>())" ref/call/implicit St
  //- MtCall=@"M<T>()" ref/call/implicit Mt
  //- @L defines/binding CtorL
  //- StCall childof CtorL
  //- MtCall childof CtorL
  L() : S<T>(M<T>()) {}
};

//- @l defines/binding VarL
//- VarL typed TAppLInt
//- TAppLInt param.0 AbsL
//- LInt specializes TAppLInt
//- CtorL childof LInt
L<int> l;
