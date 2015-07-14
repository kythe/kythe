// Checks that structs with fields that point to other structs are indexed.
//- @rat defines StructRat
struct rat {
//- @dev defines FieldDev
//- FieldDev childof StructRat
//- FieldDev.node/kind variable
//- FieldDev.subkind field
//- @fwd ref StructFwd
//- StructFwd named vname("fwd#c",_,_,_,_)
  struct fwd *dev;
};
