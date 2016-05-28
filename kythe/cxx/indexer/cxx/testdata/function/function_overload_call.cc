// Checks that calls to function overloads are recorded.
//- @F defines/binding FnFI
void F(int X) { }
//- @F defines/binding FnFF
void F(float X) {
//- @"F((int)X)" ref/call FnFI
//- !{@"F((int)X)" ref/call FnFF}
  F((int)X);
}
