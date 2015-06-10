// Checks that a struct with operator() is recorded as a callable.
//- @S defines StructS
struct S {
//- @"operator()" defines FnC
  void operator()(int A) { }
};
//- FnC childof StructS
//- FnC callableas CC
//- StructS callableas CC
