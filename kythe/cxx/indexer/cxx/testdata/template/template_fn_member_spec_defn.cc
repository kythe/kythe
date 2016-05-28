// Checks indexing refs and defs of member function specializations.
//- @S defines/binding AbsS
template <typename T> struct S { int f() { return 0; } };

int q() {
  //- @s defines/binding VarS
  //- VarS typed TAppAbsSInt
  //- TAppAbsSInt param.0 AbsS
  S<int> s;
  //- @f ref MemF
  //- MemF childof SInstInt
  //- SInstInt specializes TAppAbsSInt
  //- @"s.f()" ref/call MemF
  return s.f();
}
