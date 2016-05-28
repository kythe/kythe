// Checks that calls to a struct with operator() are correctly attributed.
// Needs --ignore_dups=true because of noncanonicalized function types.
//- @S defines/binding StructS
struct S {
//- @"operator()" defines/binding ParensF
  void operator()(int A) { }
};

void F() {
//- @"S()(42)" ref/call ParensF
  S()(42);
}
