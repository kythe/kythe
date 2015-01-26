// Checks that the indexer finds and emits nodes for types and typedefs.
//- @tdef defines TypeAlias
//- @"int" ref IntType
//- TypeAlias.node/kind talias
//- TypeAlias aliases IntType
//- TypeAlias aliases vname("int#builtin", "", "", "", "c++")
//- TypeAlias named vname("tdef#n", "", "", "", "c++")
typedef int tdef;
