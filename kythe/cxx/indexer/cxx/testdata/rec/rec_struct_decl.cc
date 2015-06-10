// Checks that forward decls are uniquely completed by same-file defs.
//- @S defines StructSNameFwd
//- StructSNameFwd named StructSName
struct S;
//- @S defines StructS
//- @S completes/uniquely StructSNameFwd
//- @S completes/uniquely StructSNameFwd2
//- StructS named StructSName
struct S { };
//- @S defines StructSNameFwd2
//- StructSNameFwd2 named StructSName
struct S;
