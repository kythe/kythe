// We associate `new Foo` with Foo's ctor.
class C {
 public:
  //- @C defines/binding Ctor
  C() { }
};
void f() {
  //- @"C()" ref/call Ctor
  //- @C ref Ctor
  new C();
  //- CallC ref/call CtorC
  //- CallC.loc/start @^"C"
  //- CallC.loc/end @^"C"
  //- @C ref Ctor
  new C;
}
