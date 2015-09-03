// Anonymous namespaces in .cc files generate @-marked names.
namespace {
class C;
//- @C ref NominalC
//- @tdef defines/binding TypeAlias
typedef C tdef;
//- TypeAlias named vname("tdef:@#n", "", "", "", "c++")
//- TypeAlias aliases NominalC
//- TypeAlias aliases vname("C:@#c#t", "", "", "", "c++")
//- NominalC.node/kind tnominal
//- NominalC named vname("C:@#c", "", "", "", "c++")
}
