// Variable writes are distinguished from reads.

int f() {
  //- @x defines/binding VarX
  int x = 3;  // Definition site.
  //- @x ref/writes VarX
  x = 4;  // Write site.
  //- @x ref VarX
  return x;  // Read site.
}
