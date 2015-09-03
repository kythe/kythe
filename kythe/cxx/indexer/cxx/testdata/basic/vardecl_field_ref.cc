// Checks that we index references to member variables.
//- @f defines/binding FieldF
struct C { int f; };
C c_inst;
//- @f ref FieldF
int i = c_inst.f;
