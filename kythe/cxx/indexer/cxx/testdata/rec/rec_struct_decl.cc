// Checks that forward decls are uniquely completed by same-file defs.
//- @S defines/binding StructSNameFwd
struct S;
//- @S defines/binding StructS
//- @S completes/uniquely StructSNameFwd
//- @S completes/uniquely StructSNameFwd2
struct S { };
//- @S defines/binding StructSNameFwd2
struct S;
