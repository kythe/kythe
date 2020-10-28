// Checks that recording influence doesn't cause problems for packs.
template<class ... Ts>
void h(int a, Ts ... ts) { }

void g() {
    //- @b defines/binding VarB
    //- @c defines/binding VarC
    //- @d defines/binding VarD
    //- @e defines/binding VarE
    //- @f defines/binding VarF
    //- @g defines/binding VarG
    int b = 1, c = 2, d = 3, e = 4, f = 5, g = 6;
    //- VarB influences _VarA
    h(b);
    //- VarC influences _VarAPrime
    //- VarD influences _
    h(c, d);
    //- VarE influences _VarAPrimePrime
    //- VarF influences _
    //- VarG influences _
    h(e, f, g);
}
