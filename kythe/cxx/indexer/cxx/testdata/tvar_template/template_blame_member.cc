// We blame member functions of class templates.
//- @g defines/binding FnG
bool g() { return false; }
//- @f defines/binding FnF
//- PtCall=@"g()" ref/call FnG
//- PtCall childof FnF
//- FnF childof CBody
//- @C defines/binding CBody
//- FImp instantiates TUnaryAppF
//- FImpCall=@"g()" ref/call/implicit FnG
//- FImpCall childof FImp
template <typename T> struct C { bool f(T* t) { return g(); } };
//- TheCall=@"j->f(nullptr)" ref/call TUnaryAppF
//- @h defines/binding FnH
//- TheCall childof FnH
//- TUnaryAppF param.0 FnF  // Until we record the whole type context: #1879
bool h(C<int>* j) { return j->f(nullptr); }
