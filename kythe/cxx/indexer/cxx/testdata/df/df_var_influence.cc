// Basic var to var influence.
void f() {
  //- @x defines/binding VarX
  //- @y defines/binding VarY
  //- @z defines/binding VarZ
  int x = 0, y = 1, z = 2;
  //- VarZ influences VarY
  //- VarY influences VarX
  //- !{VarZ influences VarX}
  //- !{VarY influences VarZ}
  x = y = z;
  //- @w defines/binding VarW
  int w = 3;
  //- VarX influences VarW
  //- VarY influences VarW
  w = x + y;
}
