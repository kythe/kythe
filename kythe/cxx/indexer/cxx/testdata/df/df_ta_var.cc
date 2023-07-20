// We see through trivial aliases to decl refs.

void f() {
  //- @x defines/binding VarX
  int x;
  //- @x ref/writes VarX
  *(&x) = 4;
}
