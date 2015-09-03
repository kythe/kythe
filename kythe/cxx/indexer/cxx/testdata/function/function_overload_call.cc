// Checks that calls to function overloads are recorded.
//- @F defines/binding FnFI
//- FnFI callableas FnFIC
void F(int X) { }
//- @F defines/binding FnFF
//- FnFF callableas FnFFC
void F(float X) {
//- @"F((int)X)" ref/call FnFIC
  F((int)X);
}
