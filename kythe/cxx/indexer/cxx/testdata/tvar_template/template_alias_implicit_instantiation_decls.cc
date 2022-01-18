// Checks that specialization decls are given the right names.
//- @S defines/binding StructS
template <typename T> struct S;
//- @T defines/binding NominalAlias
//- NominalAlias.node/kind talias
using T = S<float>;
//- NominalAlias aliases TApp
//- TApp param.0 StructS
//- StructS.node/kind record
