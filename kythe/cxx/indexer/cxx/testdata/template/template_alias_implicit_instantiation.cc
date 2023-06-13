// Checks that specialization decls (not defns) generate specializes edges.
template <typename T> struct S { };
//- @T defines/binding TAlias
using T = S<float>;
//- TAlias aliases SFloatTy
//- SFloatSpec specializes SFloatTy
//- SFloatTy.node/kind tapp
//- SFloatSpec.node/kind record
// NB: T aliases the type application of S to float, _not_ the record that
// results from the implicit instantiation of S<float>. Is this the behavior
// we want?
