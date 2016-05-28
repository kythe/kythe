// Checks that we correctly attribute initialization calls to templates.
// (This is the same as function_ctor_default_and_init, except we check that
// we do the right thing inside template bodies.)
//- @foo defines/binding FnFoo
int foo() { return 0; }
//- @bar defines/binding FnBar
int bar() { return 0; }
template <typename T>
class C {
  //- @C defines/binding CtorC0
  //- FooCall=@"foo()" ref/call FnFoo
  //- FooCall childof CtorC0
  //- !{ BarCall childof CtorC0 }
  C() : i(foo()) { }
  //- @C defines/binding CtorC1
  //- !{ FooCall childof CtorC1 }
  //- BarCall childof CtorC1
  C(int) { }
  //- @C defines/binding CtorC2
  //- !{ FooCall childof CtorC2 }
  //- BarCall childof CtorC2
  C(float) { }
  //- BarCall=@"bar()" ref/call FnBar
  int i = bar();
};

template <typename T>
class C<T*> {
  //- @C defines/binding CtorCP0
  //- FooCallP=@"foo()" ref/call FnFoo
  //- FooCallP childof CtorCP0
  //- !{ BarCallP childof CtorCP0 }
  C() : i(foo()) { }
  //- @C defines/binding CtorCP1
  //- !{ FooCallP childof CtorCP1 }
  //- BarCallP childof CtorCP1
  C(int) { }
  //- @C defines/binding CtorCP2
  //- !{ FooCallP childof CtorCP2 }
  //- BarCallP childof CtorCP2
  C(float) { }
  //- BarCallP=@"bar()" ref/call FnBar
  int i = bar();
};

template <>
class C<int> {
  //- @C defines/binding CtorCT0
  //- FooCallT=@"foo()" ref/call FnFoo
  //- FooCallT childof CtorCT0
  //- !{ BarCallT childof CtorCT0 }
  C() : i(foo()) { }
  //- @C defines/binding CtorCT1
  //- !{ FooCallT childof CtorCT1 }
  //- BarCallT childof CtorCT1
  C(int) { }
  //- @C defines/binding CtorCT2
  //- !{ FooCallT childof CtorCT2 }
  //- BarCallT childof CtorCT2
  C(float) { }
  //- BarCallT=@"bar()" ref/call FnBar
  int i = bar();
};
