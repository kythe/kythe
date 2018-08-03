// Verify that function calls inside lambdas inside macros
// include correct anchor spans.
#define CHECK(x) (x)

//- @Call defines/binding CallDecl
void Call();

void fn() {
  CHECK([] {
    //- @Call ref CallDecl
    //- @"Call()" ref/call CallDecl
    Call();
  });
}
