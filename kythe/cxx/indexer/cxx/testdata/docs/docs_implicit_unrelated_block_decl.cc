// We don't associate unrelated implicit BCPL-style comments.

//- @:7j defines/binding VarJ
//- @:8x defines/binding VarX

void f() {
  int j; // unrelated
  int x = 1;
}

//- VarX.node/kind variable
//- VarJ.node/kind variable
//- VarJD documents VarJ
//- !{VarXD documents VarX}
