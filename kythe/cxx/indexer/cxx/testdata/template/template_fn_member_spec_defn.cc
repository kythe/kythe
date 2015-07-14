// Checks indexing refs and defs of member function specializations.
//- @S defines AbsS
template <typename T> struct S { int f() { return 0; } };

int q() {
  //- @s defines VarS
  //- VarS typed TAppAbsSInt
  //- TAppAbsSInt param.0 AbsS
  S<int> s;
  //- @f ref MemF
  //- MemF childof SInstInt
  //- SInstInt specializes TAppAbsSInt
  //- MemF callableas CMemF
  //- @"s.f()" ref/call CMemF
  return s.f();
}
