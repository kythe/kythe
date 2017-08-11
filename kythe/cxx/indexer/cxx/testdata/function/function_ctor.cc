// Constructors are indexed.
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
  C(int) { }
};
void g() {
  //- @c ref/call CtorC
  C c;
  //- @"C(42)" ref/call CtorC2
  //- @C ref CtorC2
  C(42);
}
