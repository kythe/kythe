// Tests that we don't emit duplicate alias nodes.
// Since Clang appears to coalesce all `using N = T` for the same N,
// we must do this with typedefs.
//- @alias defines/binding TypeAlias
//- @"int" ref IntType
typedef int alias;
//- @alias defines/binding TypeAlias
//- @"int" ref IntType
typedef int alias;
//- TypeAlias.node/kind talias
//- TypeAlias aliases IntType
//- TypeAlias aliases vname("int#builtin", "", "", "", "c++")
