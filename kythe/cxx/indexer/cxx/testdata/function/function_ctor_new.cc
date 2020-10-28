// We associate `new Foo` with Foo's ctor.
namespace ns {
//- @C defines/binding ClassC
class C {
 public:
  //- @C defines/binding Ctor
  C() { }
};
void f() {
  //- @"C()" ref/call Ctor
  //- @C ref Ctor
  //- @C ref/id ClassC
  //- !{ @C ref ClassC }
  new C();
  //- @C ref/call Ctor
  //- @C ref Ctor
  //- @C ref/id ClassC
  //- !{ @C ref ClassC }
  new C;

  //- @"::ns::C()" ref/call Ctor
  //- @C ref Ctor
  //- @C ref/id ClassC
  //- !{ @C ref ClassC }
  new ::ns::C();
  //- @"::ns::C" ref/call Ctor
  //- @C ref Ctor
  //- @C ref/id ClassC
  //- !{ @C ref ClassC }
  new ::ns::C;
}
}
