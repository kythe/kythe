// Checks that structs with fields that point to other structs are indexed.
//- @rat defines/binding StructRat
struct rat {
//- @dev defines/binding FieldDev
//- FieldDev childof StructRat
//- FieldDev.node/kind variable
//- FieldDev.subkind field
//- @fwd ref StructFwd
//- StructFwd.node/kind tnominal
  struct fwd *dev;
};
