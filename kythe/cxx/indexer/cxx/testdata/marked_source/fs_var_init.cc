void f(int* z) {
  //- @x defines/binding VarX
  //- VarX typed TyX
  //- VarX typed/init TyX
  //- TyX.code/flat "int *"
  auto x = z;
}
