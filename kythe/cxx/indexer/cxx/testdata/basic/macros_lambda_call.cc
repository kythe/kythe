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
