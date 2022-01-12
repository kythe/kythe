// Checks that we index references to member variables of templates.
//- @f defines/binding PField
//- @S defines/binding SBody
//- SBody.node/kind record
//- PField childof SBody
template <typename T> struct S { int f; };
//- @t_int defines/binding TInt
//- TInt typed SInt
S<int> t_int;
//- @f ref Field
//- Field childof SIntImp
//- SIntImp specializes SInt
int i = t_int.f;
