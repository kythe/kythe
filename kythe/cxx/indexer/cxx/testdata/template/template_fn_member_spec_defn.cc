// Checks indexing refs and defs of member function specializations.
//- @S defines/binding AbsS
//- @f defines/binding FnF
template <typename T> struct S { int f() { return 0; } };

int q() {
  //- @s defines/binding VarS
  //- VarS typed TAppAbsSInt
  //- TAppAbsSInt param.0 AbsS
  S<int> s;
  //- @f ref UnaryTAppF  // Until we index the full type context: #1879
  //- UnaryTAppF.node/kind tapp
  //- UnaryTAppF param.0 FnF
  //- FnFImp instantiates UnaryTAppF
  //- SInstInt specializes TAppAbsSInt
  //- FnFImp childof SInstInt
  //- @"s.f()" ref/call UnaryTAppF
  return s.f();
}
