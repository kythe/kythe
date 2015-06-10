// Checks that calls to function overloads are recorded.
//- @F defines FnFI
//- FnFI callableas FnFIC
void F(int X) { }
//- @F defines FnFF
//- FnFF callableas FnFFC
void F(float X) {
//- @"F((int)X)" ref/call FnFIC
  F((int)X);
}
