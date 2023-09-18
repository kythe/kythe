// We support decayed types.
void i() {
  //- @i ref FnI
  //- FnI typed TyI
  //- @j defines/binding VarJ
  //- VarJ typed TyJ
  auto j = i;
  //- @k defines/binding VarK
  //- VarK typed TyK
  int k[1] = {1};
  //- @l defines/binding VarL
  //- VarL typed TyL
  auto* l = k;
}
//- @TFnVoidVoid defines/binding AliasFVV
//- AliasFVV aliases TyI
using TFnVoidVoid = decltype(i);
//- @TPtrFnVoidVoid defines/binding AliasPFVV
//- AliasPFVV aliases TyJ
using TPtrFnVoidVoid = decltype(i)*;
//- @TCArrInt defines/binding AliasCAI
//- AliasCAI aliases TyK
using TCArrInt = int[1];
//- @TPtrInt defines/binding AliasPI
//- AliasPI aliases TyL
using TPtrInt = int*;
