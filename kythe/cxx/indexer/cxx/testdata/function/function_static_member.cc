// Basic static member function decls are indexed.
//- @f defines/binding MemberF
//- @S defines/binding StructS
//- MemberF childof StructS
//- MemberF callableas CF
struct S { static int f() { return 0; } };
void f() {
  //- @f ref MemberF
  //- @"S::f()" ref/call CF
  int x = S::f();
}
