// Checks that calls to a struct with operator() are correctly attributed.
// Needs --ignore_dups=true because of noncanonicalized function types.
//- @S defines StructS
//- StructS callableas CC
struct S {
  void operator()(int A) { }
};

void F() {
//- @"S()(42)" ref/call CC
  S()(42);
}
