// Checks that calls to a struct with operator()s are correctly attributed.
// Needs --ignore_dups=true because of noncanonicalized function types.
//- @S defines StructS
//- StructS callableas CC
struct S {
//- @"operator()" defines FnI
//- FnI callableas CC
  void operator()(int A) { }
  void operator()(float F) { }
};

void F() {
//- @"S()(42)" ref/call CC
  S()(42);
}
