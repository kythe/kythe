// Fields are included in the influence graph.
struct S {
  //- @x defines/binding VarX
  int x = 0;
  //- @y defines/binding VarY
  //- VarX influences VarY
  int y = x;  // desugars to this->x
};
//- @z defines/binding VarZ
void f(S s, int z) {
  //- @x ref/writes VarX
  //- VarZ influences VarX
  s.x = z;
}
