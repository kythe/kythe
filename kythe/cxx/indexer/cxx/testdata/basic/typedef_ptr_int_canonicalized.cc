// Checks that types are canonicalized (eg, all int* are the same).
//- @tdef defines Tdef
typedef int* tdef;
//- @deft defines Deft
typedef int* deft;
//- Tdef aliases IntPtrType
//- Deft aliases IntPtrType