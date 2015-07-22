// We don't associate unrelated implicit BCPL-style comments.

void f() {
  int j; // unrelated
  int x = 1;
}

//- VarX.node/kind variable
//- VarX named vname("x:1:0:f#n",_,_,_,_)
//- VarJ.node/kind variable
//- VarJ named vname("j:0:0:f#n",_,_,_,_)
//- VarJD documents VarJ
//- !{VarXD documents VarX}
