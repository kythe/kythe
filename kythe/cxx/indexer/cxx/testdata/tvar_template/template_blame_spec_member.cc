// We blame member functions of total specializations.
//- @g defines/binding FnG
bool g() { return false; }
//- PtCall=@"g()" ref/call FnG
//- PtCall childof FnF  // We should collapse K in abs(K) into cluster abs(K)
//- FnF childof CBody
//- @C defines/binding CBody
template <typename T> struct C { bool f(T* t) { return g(); } };

//- @f defines/binding SpecF
//- TsCall=@"g()" ref/call FnG
//- TsCall childof SpecF
//- SpecF.code _
template <> struct C<int> { bool f(int* t) { return g(); } };

//- TheCall=@"j->f(nullptr)" ref/call SpecF
//- @h defines/binding FnH
//- TheCall childof FnH
bool h(C<int>* j) { return j->f(nullptr); }
