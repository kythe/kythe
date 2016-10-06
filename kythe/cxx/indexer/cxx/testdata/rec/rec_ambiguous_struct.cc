//- @Ambiguous defines/binding AmbiStruct
//- AmbiStruct.complete definition
//- AmbiStruct.subkind struct
//- AmbiStruct.node/kind record
struct Ambiguous {
  int field;
//- @Ambiguous defines/binding AmbiVar
//- AmbiVar.node/kind variable
//- AmbiVar typed AmbiStruct
} Ambiguous;
