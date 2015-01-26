// Checks that the indexer finds and emits nodes for types and typedefs.
class C {
  class D;
//- @D ref NominalD
//- @tdef defines TypeAlias
  typedef D tdef;
};
// Note that the tag at the end of a stringified NameId refers to the whole
// name, not just that node.
//- TypeAlias named vname("tdef:C#n", "", "", "", "c++")
//- TypeAlias aliases NominalD
//- NominalD.node/kind tnominal
//- NominalD named vname("D:C#c", "", "",
//-     "", "c++")