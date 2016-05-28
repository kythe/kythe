// Basic static member function decls are indexed.
//- @f defines/binding MemberF
//- @S defines/binding StructS
//- MemberF childof StructS
struct S { static int f() { return 0; } };
void f() {
  //- @f ref MemberF
  //- @"S::f()" ref/call MemberF
  int x = S::f();
}
