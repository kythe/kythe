// We blame member functions of class templates.
//- @g defines/binding FnG
bool g() { return false; }
//- @f defines/binding AbsF
//- PtCall=@"g()" ref/call FnG
//- PtCall childof FnF  // We should collapse K in abs(K) into cluster abs(K)
//- FnF childof CBody
//- CBody childof AbsC
//- @C defines/binding AbsC
//- FImp instantiates TUnaryAppAbsF
//- FImpCall=@"g()" ref/call/implicit FnG
//- FImpCall childof FImp
template <typename T> struct C { bool f(T* t) { return g(); } };
//- TheCall=@"j->f(nullptr)" ref/call TUnaryAppAbsF
//- @h defines/binding FnH
//- TheCall childof FnH
//- TUnaryAppAbsF param.0 AbsF  // Until we record the whole type context: #1879
bool h(C<int>* j) { return j->f(nullptr); }
