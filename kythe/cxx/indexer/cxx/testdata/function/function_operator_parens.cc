// Checks that a struct with operator() is recorded as a callable.
//- @S defines/binding StructS
struct S {
//- @"operator()" defines/binding FnC
  void operator()(int A) { }
};
//- FnC childof StructS
//- FnC callableas CC
//- StructS callableas CC
