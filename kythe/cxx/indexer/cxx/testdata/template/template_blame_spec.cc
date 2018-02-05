// We blame function specializations.
//- @g defines/binding FnG
bool g() { return false; }
//- @f defines/binding AbsF
//- PtCall=@"g()" ref/call FnG
//- PtCall childof FnF  // We should collapse K in abs(K) into cluster abs(K)
//- FnF childof AbsF
//- FIntImp instantiates TAppAbsFInt
//- FIntImpCall=@"g()" ref/call/implicit FnG  // Aliasing is off by default.
//- FIntImpCall childof FIntImp
template <typename T> bool f(T* t) { return g(); }
//- TheCall=@"f(&i)" ref/call TAppAbsFInt
//- @h defines/binding FnH
//- TheCall childof FnH
//- TAppAbsFInt param.0 AbsF
//- TAppAbsFInt param.1 Int
//- @int ref Int
bool h(int i) { return f(&i); }
