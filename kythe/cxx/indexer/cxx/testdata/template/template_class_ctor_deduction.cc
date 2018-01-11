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

//- @#1C ref TemplC
template <typename U> C(U, U) -> C<U>;

void f() {
  // TODO(shahms): Uncomment this test when Clang supports class template
  // argument deduction for default construction.
  // See: https://bugs.llvm.org/show_bug.cgi?id=32443
  // C cz;
  C ct(S{});
  C cu(S{}, S{});
}
