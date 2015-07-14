// We associate `new Foo` with Foo's ctor.
class C {
 public:
  //- @C defines Ctor
  //- Ctor callableas CtorC
  C() { }
};
void f() {
  //- @"C()" ref/call CtorC
  new C();
}
