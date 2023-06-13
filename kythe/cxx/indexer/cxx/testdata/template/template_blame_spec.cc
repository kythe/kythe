// We blame function specializations.
//- @g defines/binding FnG
bool g() { return false; }
//- @f defines/binding FnF
//- PtCall=@"g()" ref/call FnG
//- PtCall childof FnF
//- FIntImp instantiates TAppFInt
//- FIntImpCall=@"g()" ref/call/implicit FnG  // Aliasing is off by default.
//- FIntImpCall childof FIntImp
template <typename T> bool f(T* t) { return g(); }
//- TheCall=@"f(&i)" ref/call TAppFInt
//- @h defines/binding FnH
//- TheCall childof FnH
//- TAppFInt param.0 FnF
//- TAppFInt param.1 Int
//- @int ref Int
bool h(int i) { return f(&i); }
