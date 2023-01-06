//- @B defines/binding StructB
//- @f defines/binding FnFBDecl
struct B { void f(); };
//- @C defines/binding StructC
//- @f defines/binding FnFCDecl
struct C : B { void f(); };

//- @C ref StructC
//- @f completes/uniquely FnFCDecl
//- @f defines/binding FnFCDefn
//- FnFCDecl completedby FnFCDefn
void C::f() {
//- @B ref StructB
//- @f ref FnFBDecl
  B::f();

  C c;
//- @B ref StructB
//- @f ref FnFBDecl
  c.B::f();
}
