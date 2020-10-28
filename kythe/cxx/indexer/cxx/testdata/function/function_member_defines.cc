// Full definition ranges for member functions are recorded.

class C {
  //- @f defines/binding FnFDecl
  void f();
  //- @g defines/binding FnG
  //- @"void g() { }" defines FnG
  void g() { }
};

//- @"void C::f() { }" defines _FnFDefn
//- @f completes/uniquely FnFDecl
void C::f() { }
