// We index records with combinations of default values and init lists.
//- @foo defines/binding FnFoo
int foo() { return 0; }
//- @bar defines/binding FnBar
int bar() { return 0; }
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
