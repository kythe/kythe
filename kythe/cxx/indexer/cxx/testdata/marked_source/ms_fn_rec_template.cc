// We emit reasonable markup for functions under record templates.
//- @f defines/binding FnF
//- FnF code _
template <typename T> struct S { void f(T t) { } };

//- @f defines/binding FnFSpec
//- FnFSpec code _
template <> struct S<int> { void f(int t) { } };

void g(S<double>& s) {
//- @f ref NullaryTAppFnF
//- !{FnFInst code _}
//- FnFInst instantiates NullaryTAppFnF
//- NullaryTAppFnF param.0 FnF
  s.f(0.0);
}
