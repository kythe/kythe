// Checks that we properly handle classes with constructor templates.
//- @foo defines FnFoo
//- FnFoo callableas FooC
int foo() { return 0; }
//- @bar defines FnBar
//- FnBar callableas BarC
int bar() { return 0; }
class C {
 public:
  template <typename T>
  //- FooCall=@"foo()" ref/call FooC
  //- FooCall childof CtorC
  //- @C defines AbsCtorC
  //- CtorC childof AbsCtorC
  //- !{ BarCall childof CtorC }
  //- BarCallJ childof CtorC
  C(T) : i(foo()) { }

  //- BarCall=@"bar()" ref/call BarC
  int i = bar();
  //- BarCallJ=@"bar()" ref/call BarC
  int j = bar();
};

template <> C::
//- FooCall2=@"foo()" ref/call FooC
//- FooCall2 childof CtorC2
//- @C defines CtorC2
//- !{ BarCall childof CtorC2 }
//- BarCallJ childof CtorC2
C(float) : i(foo()) { }
