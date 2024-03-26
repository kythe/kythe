void f(int* z) {
  //- @x defines/binding VarX
  //- VarX typed TyX
  //- VarX exp/typed/init TyX
  //- TyX.exp/code/flat "int *"
  auto x = z;
}
