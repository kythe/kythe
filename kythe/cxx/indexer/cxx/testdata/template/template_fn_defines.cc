// Full definition ranges for function templates are recorded.

//- @"template <typename T> void f(T t) { }" defines TFnF
//- @f defines/binding TFnF
template <typename T> void f(T t) { }

//- @f defines/binding TFnFTS
//- @"template <> void f(int t) { }" defines TFnFTS
template <> void f(int t) { }

//- @f defines/binding TFnFPS
//- @"template <typename S> void f(S* s) { }" defines TFnFPS
template <typename S> void f(S* s) { }

class C {
  //- @f defines/binding TMFn
  //- @"template <typename T> void f(T t) { }" defines TMFn
  template <typename T> void f(T t) { }

  template <typename T> void g();
};

//- @g defines/binding TMFnExt
//- @"template <typename T> void C::g() { }" defines TMFnExt
template <typename T> void C::g() { }
