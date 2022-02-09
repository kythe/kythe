// Checks that we properly handle classes with constructor templates.
//- @foo defines/binding FnFoo
int foo() { return 0; }
//- @bar defines/binding FnBar
int bar() { return 0; }
class C {
 public:
  template <typename T>
  //- FooCall=@"foo()" ref/call FnFoo
  //- FooCall childof CtorC
  //- @C defines/binding CtorC
  //- !{ BarCall childof CtorC }
  //- BarCallJ childof CtorC
  C(T) : i(foo()) { }

  //- BarCall=@"bar()" ref/call FnBar
  int i = bar();
  //- BarCallJ=@"bar()" ref/call FnBar
  int j = bar();
};

template <> C::
//- FooCall2=@"foo()" ref/call FnFoo
//- FooCall2 childof CtorC2
//- @C defines/binding CtorC2
//- !{ BarCall childof CtorC2 }
//- BarCallJ childof CtorC2
C(float) : i(foo()) { }
