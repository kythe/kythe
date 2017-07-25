// Checks that the indexer finds and emits nodes for types and typedefs.
//- @tdef defines/binding TypeAlias
//- TypeAlias.node/kind talias
//- @int ref IntType
typedef const int tdef;
//- TypeAlias aliases _ConstIntType
//- IntPtrType.node/kind tapp
//- IntPtrType param.0 vname("const#builtin", "", "", "", "c++")
//- IntPtrType param.1 IntType
//- IntPtrType param.1 vname("int#builtin", "", "", "", "c++")
