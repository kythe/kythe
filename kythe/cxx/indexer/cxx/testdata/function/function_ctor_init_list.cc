// We index elements of initializer lists.
//- @foo defines/binding FnFoo
int foo() { return 0; }
class B {
 public:
  //- @B defines/binding BCtor
  B(int x) { }
};
class C : public B {
  //- BFooCall=@"foo()" ref/call FnFoo
  //- BCtorCall=@"B(foo())" ref/call BCtor
  //- BFooCall childof CtorC
  //- BCtorCall childof CtorC
  //- @C defines/binding CtorC
  C() : B(foo()),
  //- IFooCall=@"foo()" ref/call FnFoo
  //- IFooCall childof CtorC
        i(foo()),
  //- LbFooCall=@"foo()" ref/call FnFoo
  //- LbCtorCall=@"b(foo())" ref/call BCtor
  //- LbFooCall childof CtorC
  //- LbCtorCall childof CtorC
        b(foo()) { }
  int i;
  B b;
};
