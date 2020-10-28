// Checks that recording influence doesn't cause problems for varargs.

//- @a defines/binding VarA
void h(int a, ...);

void g() {
    //- @b defines/binding VarB
    //- @c defines/binding VarC
    //- @d defines/binding VarD
    //- @e defines/binding VarE
    //- @f defines/binding VarF
    //- @g defines/binding VarG
    int b = 1, c = 2, d = 3, e = 4, f = 5, g = 6;
    //- VarB influences VarA
    h(b);
    //- !{VarC influences VarA}
    //- !{VarD influences _}
    h(c, d);
    //- !{VarE influences VarA}
    //- !{VarF influences _}
    //- !{VarG influences _}
    h(e, f, g);
}
