// We emit reasonable markup for variables in function templates.
//- @f defines/binding FnF
//- FnF code _
//- @param defines/binding FnFT
//- FnFT code PCRoot
template <typename T> void f(T param) { }

//- @f defines/binding FnFSpec
//- FnFSpec code _
//- FnFSpec param.0 FnFSpecVar
//- FnFSpecVar code _
//- @param defines/binding FnFSpecT
//- FnFSpecT code SCRoot
template <> void f(int param) { }

void g() {
//- @f ref FnFApp
//- !{FnFInst code _}
//- FnFInst instantiates FnFApp
//- FnFApp param.0 FnF
//- FnFInst param.0 InstVar
//- !{InstVar code _}
  f(nullptr);
}
