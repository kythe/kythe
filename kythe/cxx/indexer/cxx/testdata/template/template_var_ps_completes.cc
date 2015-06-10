// Checks completion edges for variable template partial specializations.
template <typename T, typename S> extern T z;
//- @z defines VarZPsAbsDecl
template <typename U> extern int z<int, U>;
//- @z completes/uniquely VarZPsAbsDecl
template <typename U> int z<int, U>;
