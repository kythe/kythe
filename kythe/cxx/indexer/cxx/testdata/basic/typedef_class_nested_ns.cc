// Nested namespaces create nominal types with paths.
namespace A {
namespace B {
class C;
}
//- @tdef defines/binding TypeAlias
//- @C ref NominalCBA
typedef B::C tdef;
}
//- TypeAlias aliases NominalCBA
//- TypeAlias aliases vname("C:B:A#c#t", "", "", "", "c++")
//- NominalCBA.node/kind tnominal
