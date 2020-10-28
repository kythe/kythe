// Checks that we correctly attribute class template instantiation
// via C++17 constructor argument deduction and guides.

//- @S defines/binding StructS
struct S {};

template <typename T>
//- @C defines/binding TemplC
struct C {

  //- @C defines/binding Ctor0
  C() { }

  //- @C defines/binding CtorT
  C(T) { }

  template <typename U>
  //- @C defines/binding CtorU
  C(U, U) { }
};

//- @#1C ref TemplC
//- @S ref StructS
C() -> C<S>;

// TODO(shahms): This should not be considered an implicit ref.
//- @#1C ref/implicit TemplC
template <typename U> C(U, U) -> C<U>;

void f() {
  //- @C ref TemplC
  //- @cz ref/call Ctor0App
  //- Ctor0App param.0 Ctor0
  C cz;
  //- @C ref TemplC
  //- @"ct(S{})" ref/call CtorTApp
  //- CtorTApp param.0 CtorT
  C ct(S{});
  //- @C ref TemplC
  //- @"cu(S{}, S{})" ref/call CtorUApp
  //- CtorUApp param.0 CtorU
  C cu(S{}, S{});
}
