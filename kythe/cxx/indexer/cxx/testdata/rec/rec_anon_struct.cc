// Checks that anonymous structs are handled properly.
// Needs -ignore_dups=true: the anonymous struct and the variable S both
// emit `uses` nodes pointing from the struct syntax to the type.
//- @struct defines/binding AnonStruct
//- AnonStruct.complete definition
//- AnonStruct.subkind struct
//- AnonStruct.node/kind record
//- @S defines/binding VarS
//- VarS typed AnonStruct
struct { } S;

// Verify that additional, structurally identical, anonymous
// structs do not share a type.
//- @T defines/binding VarT
//- !{ VarT typed AnonStruct }
struct { } T;
