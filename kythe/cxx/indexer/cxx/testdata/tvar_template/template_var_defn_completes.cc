// Checks that variable template defns complete variable template decls.
//- @z defines/binding VarZDecl
template <typename T> extern T z;
//- @z completes/uniquely VarZDecl
template <typename T> T z;
