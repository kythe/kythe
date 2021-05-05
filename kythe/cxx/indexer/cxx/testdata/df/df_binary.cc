// Accumulator-style operators are in the influence graph.

void f(int x, int y, int z) {
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
    //- VarX influences VarX
    //- VarZ influences VarZ
    //- VarY influences VarX
    //- VarX influences VarZ
}
