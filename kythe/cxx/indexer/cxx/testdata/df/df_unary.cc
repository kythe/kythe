// Unary {post,pre}{inc,dec}rements are included in the influence graph.
void f() {
    int x;
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
}
