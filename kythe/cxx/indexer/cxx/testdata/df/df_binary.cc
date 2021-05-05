// Accumulator-style operators are in the influence graph.

void f(int x, int y, int z, int j, int k) {
  //- @x ref VarX
  //- @x ref/writes VarX
  //- @y ref VarY
  //- !{ @y ref/writes VarY }
  x += y;
  //- @z ref VarZ
  //- @z ref/writes VarZ
  //- @x ref VarX
  //- !{ @x ref/writes VarX }
  z -= x;
  //- @j ref VarJ
  //- @j ref/writes VarJ
  //- @k ref VarK
  //- @k ref/writes VarK
  j -= k++;
  //- VarX influences VarX
  //- VarZ influences VarZ
  //- VarY influences VarX
  //- VarX influences VarZ
  //- VarK influences VarK
  //- VarJ influences VarJ
  //- VarK influences VarJ
}
