// Checks completion edges for variable template partial specializations.
template <typename T, typename S> extern T z;
//- @z defines/binding VarZPsAbsDecl
template <typename U> extern int z<int, U>;
//- @z defines/binding VarZPsAbsDefn
//- VarzPsAbsDecl completedby VarZPsAbsDefn
template <typename U> int z<int, U>;
