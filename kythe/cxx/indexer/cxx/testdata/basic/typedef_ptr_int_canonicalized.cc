// Checks that types are canonicalized (eg, all int* are the same).
//- @tdef defines/binding Tdef
typedef int* tdef;
//- @deft defines/binding Deft
typedef int* deft;
//- Tdef aliases IntPtrType
//- Deft aliases IntPtrType