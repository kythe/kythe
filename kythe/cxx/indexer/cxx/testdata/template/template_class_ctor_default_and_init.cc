// Checks that we correctly attribute initialization calls to templates.
// (This is the same as function_ctor_default_and_init, except we check that
// we do the right thing inside template bodies.)
//- @foo defines FnFoo
//- FnFoo callableas FooC
int foo() { return 0; }
//- @bar defines FnBar
//- FnBar callableas BarC
int bar() { return 0; }
template <typename T>
class C {
  //- @C defines CtorC0
  //- FooCall=@"foo()" ref/call FooC
  //- FooCall childof CtorC0
  //- !{ BarCall childof CtorC0 }
  C() : i(foo()) { }
  //- @C defines CtorC1
  //- !{ FooCall childof CtorC1 }
  //- BarCall childof CtorC1
  C(int) { }
  //- @C defines CtorC2
  //- !{ FooCall childof CtorC2 }
  //- BarCall childof CtorC2
  C(float) { }
  //- BarCall=@"bar()" ref/call BarC
  int i = bar();
};

template <typename T>
class C<T*> {
  //- @C defines CtorCP0
  //- FooCallP=@"foo()" ref/call FooC
  //- FooCallP childof CtorCP0
  //- !{ BarCallP childof CtorCP0 }
  C() : i(foo()) { }
  //- @C defines CtorCP1
  //- !{ FooCallP childof CtorCP1 }
  //- BarCallP childof CtorCP1
  C(int) { }
  //- @C defines CtorCP2
  //- !{ FooCallP childof CtorCP2 }
  //- BarCallP childof CtorCP2
  C(float) { }
  //- BarCallP=@"bar()" ref/call BarC
  int i = bar();
};

template <>
class C<int> {
  //- @C defines CtorCT0
  //- FooCallT=@"foo()" ref/call FooC
  //- FooCallT childof CtorCT0
  //- !{ BarCallT childof CtorCT0 }
  C() : i(foo()) { }
  //- @C defines CtorCT1
  //- !{ FooCallT childof CtorCT1 }
  //- BarCallT childof CtorCT1
  C(int) { }
  //- @C defines CtorCT2
  //- !{ FooCallT childof CtorCT2 }
  //- BarCallT childof CtorCT2
  C(float) { }
  //- BarCallT=@"bar()" ref/call BarC
  int i = bar();
};
