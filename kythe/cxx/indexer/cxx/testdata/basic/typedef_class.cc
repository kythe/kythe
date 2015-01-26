// Checks that the indexer finds and emits nodes for types and typedefs.
class C { };
//- @C ref NominalC
//- @tdef defines TypeAlias
typedef C tdef;
//- TypeAlias named vname("tdef#n", "", "", "", "c++")
//- TypeAlias aliases DefnC
//- DefnC.node/kind record
//- DefnC named vname("C#c", "", "", "", "c++")