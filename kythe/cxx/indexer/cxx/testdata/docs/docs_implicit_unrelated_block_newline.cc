// We don't associate unrelated implicit BCPL-style comments with newlines.

void f() {
  if (true) return;  // unrelated

  int x = 1;
}

//- VarX.node/kind variable
//- !{Something documents VarX}
