// We index records with combinations of default values and init lists.
//- @foo defines FnFoo
//- FnFoo callableas FooC
int foo() { return 0; }
//- @bar defines FnBar
//- FnBar callableas BarC
int bar() { return 0; }
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
