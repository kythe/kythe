// Checks that enumerations complete forward declarations (with types).
//- @E defines EEnumFwdT
//- EEnumFwdT.complete complete
//- EEnumFwdT named EEnumName
//- EEnumFwdT typed ShortType
enum class E : short;
//- @E defines EEnum
//- EEnum.complete definition
//- @E completes/uniquely EEnumFwdT
//- EEnum typed ShortType
//- EEnum named EEnumName
enum class E : short { };
