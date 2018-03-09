// Constructors are indexed.

namespace ns {
//- @f defines/binding FnF
void f() { }
//- @C defines/binding ClassC
class C {
 public:
  //- @C defines/binding CtorC
  //- CtorC childof ClassC
  //- CtorC.node/kind function
  //- CtorC.subkind constructor
  //- Call ref/call FnF
  //- Call childof CtorC
  C() { f(); }
  //- @C defines/binding CtorC2
  //- CtorC2 childof ClassC
  C(int, void* = nullptr) { }
};
void g() {
  //- @c ref/call CtorC
  C c;

  //- @"C(42)" ref/call CtorC2
  //- @C ref CtorC2
  C(42);

  //- @"::ns::C(42)" ref/call CtorC2
  //- @C ref CtorC2
  ::ns::C(42);

  //- @"::ns::C(42, nullptr)" ref/call CtorC2
  //- @C ref CtorC2
  ::ns::C(42, nullptr);
}
}
