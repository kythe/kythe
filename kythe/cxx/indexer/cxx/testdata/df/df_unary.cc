// Unary {post,pre}{inc,dec}rements are included in the influence graph.
void f() {
    int x, y, z;
    //- @x ref VarX
    //- @x ref/writes VarX
    x++;
    //- @x ref VarX
    //- @x ref/writes VarX
    ++x;
    //- @x ref VarX
    //- @x ref/writes VarX
    x--;
    //- @x ref VarX
    //- @x ref/writes VarX
    --x;
    //- VarX influences VarX
    //- !{ @x ref/writes VarX }
    //- !{ @z ref/writes VarZ }
    //- // !{ @y ref VarY } todo(zarko): transitional change.
    //- @z ref VarZ
    //- @x ref VarX
    //- @y ref/writes VarY
    y = x + z;
}
