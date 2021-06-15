// A basic test which includes the VerifyDiagnosticConsumer
// which has caused use-after-free issues in the past.
// See https://github.com/kythe/kythe/pull/4775

//- @fn defines/binding FuncDef
void fn() {
  //- @fn ref FuncDef
  fn(); // expected-no-diagnostics
}
