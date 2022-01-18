// We blame member functions of partial specializations.
//- @g defines/binding FnG
bool g() { return false; }
//- PtCall=@"g()" ref/call FnG
//- PtCall childof FnF
//- FnF childof CBody
//- @C defines/binding CBody
template <typename T, typename S> struct C { bool f(T* t) { return g(); } };

//- TsCall childof SpecF
//- TsCall=@"g()" ref/call FnG
//- SpecF.code _
//- @f defines/binding SpecF
//- TsImp instantiates TAppUnarySpecF
//- TsImpCall=@"g()" ref/call/implicit FnG
//- TsImpCall childof TsImp  // Aliasing is off by default
template <typename T> struct C<T, int> { bool f(T* t) { return g(); } };

// Until type contexts: #1879
//- TheCall=@"j->f(nullptr)" ref/call TAppUnarySpecF
//- @h defines/binding FnH
//- TheCall childof FnH
//- TAppUnarySpecF param.0 SpecF
bool h(C<int, int>* j) { return j->f(nullptr); }
