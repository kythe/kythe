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
  //- @C ref/call Ctor
  //- @C ref Ctor
  new C;
}
