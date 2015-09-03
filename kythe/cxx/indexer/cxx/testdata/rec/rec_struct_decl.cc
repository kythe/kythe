// Checks that forward decls are uniquely completed by same-file defs.
//- @S defines/binding StructSNameFwd
//- StructSNameFwd named StructSName
struct S;
//- @S defines/binding StructS
//- @S completes/uniquely StructSNameFwd
//- @S completes/uniquely StructSNameFwd2
//- StructS named StructSName
struct S { };
//- @S defines/binding StructSNameFwd2
//- StructSNameFwd2 named StructSName
struct S;
