// We index elements of initializer lists.
//- @foo defines FnFoo
//- FnFoo callableas FnFooC
int foo() { return 0; }
class B {
 public:
  //- @B defines BCtor
  //- BCtor callableas BCtorC
  B(int x) { }
};
class C : public B {
  //- BFooCall=@"foo()" ref/call FnFooC
  //- BCtorCall=@"B(foo())" ref/call BCtorC
  //- BFooCall childof CtorC
  //- BCtorCall childof CtorC
  //- @C defines CtorC
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
