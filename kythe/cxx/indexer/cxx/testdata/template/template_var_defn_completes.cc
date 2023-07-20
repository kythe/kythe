// Checks that variable template defns complete variable template decls.
//- @z defines/binding VarZDecl
template <typename T> extern T z;
//- @z defines/binding VarZDefn
//- VarZDecl completedby VarZDefn
template <typename T> T z;
