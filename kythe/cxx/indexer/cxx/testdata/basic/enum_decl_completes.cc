// Checks that enumerations complete forward declarations.
//- @E defines/binding EEnumFwd
//- EEnumFwd.node/kind sum
//- EEnumFwd.complete incomplete
//- EEnumFwd.subkind enumClass
enum class E;
//- @E defines/binding EEnum
//- EEnum.complete definition
//- EEnumFwd completedby EEnum
enum class E { };
