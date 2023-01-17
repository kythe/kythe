// Checks that forward decls are uniquely completed by same-file defs.
//- @S defines/binding StructSNameFwd
struct S;
//- @S defines/binding StructS
//- StructSNameFwd completedby StructS
//- StructSNameFwd2 completedby StructS
struct S { };
//- @S defines/binding StructSNameFwd2
struct S;
