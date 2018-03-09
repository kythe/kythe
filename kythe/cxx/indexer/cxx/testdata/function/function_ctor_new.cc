// We associate `new Foo` with Foo's ctor.
namespace ns {
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

  //- @"::ns::C()" ref/call Ctor
  //- @C ref Ctor
  new ::ns::C();
  //- @"::ns::C" ref/call Ctor
  //- @C ref Ctor
  new ::ns::C;
}
}
