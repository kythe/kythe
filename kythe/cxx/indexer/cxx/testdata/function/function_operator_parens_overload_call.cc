// Checks that calls to a struct with operator()s are correctly attributed.
// Needs --ignore_dups=true because of noncanonicalized function types.
//- @S defines/binding StructS
struct S {
//- @"operator()" defines/binding FnI
  void operator()(int A) { }
  void operator()(float F) { }
};

void F() {
//- @"S()(42)" ref/call FnI
  S()(42);
}
