// Checks enum decls with types.
//- @E defines/binding ECEnum
//- ECEnum.complete definition
//- ECEnum typed ShortType
enum class E : short { };
//- @EE defines/binding EEnum
//- EEnum.complete definition
//- EEnum typed ShortType
enum EE : short { };
