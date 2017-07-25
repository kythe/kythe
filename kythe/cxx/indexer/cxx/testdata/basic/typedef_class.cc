// Checks that the indexer finds and emits nodes for types and typedefs.
class C { };
//- @C ref DefnC
//- @tdef defines/binding TypeAlias
typedef C tdef;
//- TypeAlias aliases DefnC
//- DefnC.node/kind record
