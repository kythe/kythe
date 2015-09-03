// We index elements of initializer lists.
//- @foo defines/binding FnFoo
//- FnFoo callableas FnFooC
int foo() { return 0; }
class B {
 public:
  //- @B defines/binding BCtor
  //- BCtor callableas BCtorC
  B(int x) { }
};
class C : public B {
  //- BFooCall=@"foo()" ref/call FnFooC
  //- BCtorCall=@"B(foo())" ref/call BCtorC
  //- BFooCall childof CtorC
  //- BCtorCall childof CtorC
  //- @C defines/binding CtorC
  C() : B(foo()),
  //- IFooCall=@"foo()" ref/call FnFooC
  //- IFooCall childof CtorC
        i(foo()),
  //- LbFooCall=@"foo()" ref/call FnFooC
  //- LbCtorCall=@"b(foo())" ref/call BCtorC
  //- LbFooCall childof CtorC
  //- LbCtorCall childof CtorC
        b(foo()) { }
  int i;
  B b;
};
