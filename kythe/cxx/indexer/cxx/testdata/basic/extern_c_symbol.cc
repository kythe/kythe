// extern "C" function definitions are given names.
//- @f defines/binding FnF
//- !{ FnF named _ }
void f();
//- @g defines/binding FnG
//- FnG named vname("g","file","","","symbol")
extern "C" void g() {}
//- @h defines/binding FnH
//- !{ FnH named _ }
extern "C" void h();
