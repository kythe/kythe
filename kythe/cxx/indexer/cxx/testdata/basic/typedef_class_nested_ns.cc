// Nested namespaces create nominal types with paths.
namespace A {
namespace B {
class C;
}
//- @tdef defines TypeAlias
//- @C ref NominalCBA
typedef B::C tdef;
}
//- TypeAlias named vname("tdef:A#n", "", "", "", "c++")
//- TypeAlias aliases NominalCBA
//- TypeAlias aliases vname("C:B:A#c#t", "", "", "", "c++")
//- NominalCBA.node/kind tnominal
//- NominalCBA named vname("C:B:A#c", "", "", "", "c++")