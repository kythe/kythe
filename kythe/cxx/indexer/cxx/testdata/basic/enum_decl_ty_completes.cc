// Checks that enumerations complete forward declarations (with types).
//- @E defines/binding EEnumFwdT
//- EEnumFwdT.complete complete
//- EEnumFwdT typed ShortType
enum class E : short;
//- @E defines/binding EEnum
//- EEnum.complete definition
//- @E completes/uniquely EEnumFwdT
//- EEnumFwdT completedby EEnum
//- EEnum typed ShortType
enum class E : short { };
