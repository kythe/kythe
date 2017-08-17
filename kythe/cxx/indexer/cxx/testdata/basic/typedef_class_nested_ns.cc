// Nested namespaces create nominal types with paths.
namespace A {
namespace B {
//- @C defines/binding DeclCBA
class C;
}
//- @tdef defines/binding TypeAlias
//- @C ref DeclCBA
typedef B::C tdef;
}
//- TypeAlias aliases NominalCBA
//- TypeAlias aliases NominalCBA=vname("C:B:A#c#t", "", "", "", "c++")
//- NominalCBA.node/kind tnominal
