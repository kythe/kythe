// Basic member function decls are indexed.
//- @f defines/binding MemberF
//- @S defines/binding StructS
//- MemberF childof StructS
//- MemberF named vname("f:S#n",_,_,_,_)
struct S { int f() { return 0; } };
void f() {
  //- @s defines/binding VarS
  S s;
  //- @s ref VarS
  //- @f ref MemberF
  //- @"s.f()" ref/call MemberF
  int x = s.f();
  //- @f ref MemberF
  //- @"(&s)->f()" ref/call MemberF
  int y = (&s)->f();
}
