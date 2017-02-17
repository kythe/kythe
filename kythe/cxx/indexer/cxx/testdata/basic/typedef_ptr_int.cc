// Checks that the indexer finds and emits nodes for types and typedefs.
//- @tdef defines/binding TypeAlias
//- TypeAlias.node/kind talias
//- TypeAlias named vname("tdef#n", "", "", "", "c++")
//- @int ref IntType
//- !{@"int*" ref _}
typedef int* tdef;
//- TypeAlias aliases IntPtrType
//- IntPtrType.node/kind tapp
//- IntPtrType param.0 vname("ptr#builtin", "", "", "", "c++")
//- IntPtrType param.1 IntType
//- IntPtrType param.1 vname("int#builtin", "", "", "", "c++")
