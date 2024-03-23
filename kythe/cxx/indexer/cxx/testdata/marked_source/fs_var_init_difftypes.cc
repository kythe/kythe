//- @A defines/binding TyA
class A {};
//- @B defines/binding TyB
class B : public A {};
void f(B* b) {
  //- @float ref TyFloat
  float z;
  //- @x defines/binding VarX
  //- @int ref TyInt
  //- VarX typed TyInt
  //- VarX exp/typed/init TyFloat
  //- TyFloat.exp/code/flat "float"
  //- !{ VarX typed TyFloat }
  int x = 3.0f;
  //- @a defines/binding VarA
  //- VarA typed TyPtrA
  //- TyPtrA param.1 TyA
  //- VarA exp/typed/init TyPtrB
  //- TyPtrB param.1 TyB
  //- TyPtrB.exp/code/flat "B *"
  A* a = b;
}
