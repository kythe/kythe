// Full definition ranges for member functions are recorded.

class C {
  //- @f defines/binding FnFDecl
  void f();
  //- @g defines/binding FnG
  //- @"void g() { }" defines FnG
  void g() { }
};

//- @"void C::f() { }" defines FnFDefn
//- @f completes/uniquely FnFDecl
//- FnFDecl completedby FnFDefn
void C::f() { }
