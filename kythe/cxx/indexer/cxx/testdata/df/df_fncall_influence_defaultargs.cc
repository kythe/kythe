// Checks that influence is recorded for function calls with default arguments.

//- @a defines/binding VarA
//- @b defines/binding VarB
void f(int a, int b = 2);

void g() {
    //- @c defines/binding VarC
    //- @d defines/binding VarD
    //- @e defines/binding VarE
    int c = 3, d = 4, e = 5;
    //- VarC influences VarA
    f(c);
    //- VarD influences VarA
    //- VarE influences VarB
    f(d, e);
}
