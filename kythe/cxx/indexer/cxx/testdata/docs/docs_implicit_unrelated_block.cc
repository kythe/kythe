// We don't associate unrelated implicit BCPL-style comments.

void f() {
  if (true) return;  // unrelated
  int x = 1;
}

//- VarX.node/kind variable
//- !{VarXD documents VarX}
