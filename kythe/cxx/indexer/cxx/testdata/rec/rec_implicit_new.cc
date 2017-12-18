// Locate implicit constructors at the site of their parent's declaration.
//- StructSDefAnchor=@S defines/binding StructS
struct S {};

//- @"S()" ref/call StructSCtor
//- StructSDefAnchor defines/binding StructSCtor
//- !{StructSDefAnchor.subkind implicit}
void f() { new S(); }
