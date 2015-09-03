// Checks that enumerations complete forward declarations (with types).
//- @E defines/binding EEnumFwdT
//- EEnumFwdT.complete complete
//- EEnumFwdT named EEnumName
//- EEnumFwdT typed ShortType
enum class E : short;
//- @E defines/binding EEnum
//- EEnum.complete definition
//- @E completes/uniquely EEnumFwdT
//- EEnum typed ShortType
//- EEnum named EEnumName
enum class E : short { };
