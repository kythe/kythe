// Writes into arrays are not writes of those arrays.

int* f() {
  //- @x defines/binding VarX
  int x[2] = {0, 1};  // Definition site.
  //- @x ref VarX
  //- !{@x ref/writes VarX}
  x[0] = 1;
  //- @x ref VarX
  //- !{@x ref/writes VarX}
  *x = 2;
  //- @x ref VarX
  return x;  // Read site.
}
