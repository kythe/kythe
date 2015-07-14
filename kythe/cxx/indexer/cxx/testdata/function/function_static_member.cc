// Basic static member function decls are indexed.
//- @f defines MemberF
//- @S defines StructS
//- MemberF childof StructS
//- MemberF callableas CF
struct S { static int f() { return 0; } };
void f() {
  //- @f ref MemberF
  //- @"S::f()" ref/call CF
  int x = S::f();
}
