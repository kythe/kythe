// Tests that the VNames assigned to various structurl nodes is the same as the
// compilation unit's.
//- vname("", "file", "", "kythe/cxx/indexer/cxx/testdata/basic/vname_corpus.cc", "").node/kind file

// Simple types get the corpus from the file.
//- @S=vname(_,"file",_,_,_) defines/binding StructDef=vname(_,"file",_,_,_)
struct S {};

//- @T defines/binding TemplateDef=vname(_,"file",_,_,_)
template <typename> struct T {};

// Structural types get the corpus from the compilation unit.
//- @use defines/binding UseDef
//- UseDef typed TApp=vname(_,"unit",_,_,_)
//- TApp.node/kind tapp
//- TApp param.0 TemplateDef
T<int> use;

// As do aliases.
//- @U defines/binding AliasDef=vname(_,"unit",_,_,_)
using U = S;

//- @named ref NamedNS=vname(_,"unit",_,_,_)
//- NamedNS.node/kind package
//- NamedNS.subkind namespace
namespace named {
namespace /* anonymous */ {
//- @C defines/binding StructC
//- StructC childof AnonNS=vname(_,"file",_,_,_)
//- AnonNS childof NamedNS
//- AnonNS.node/kind package
//- AnonNS.subkind namespace
struct C {};
}
}
