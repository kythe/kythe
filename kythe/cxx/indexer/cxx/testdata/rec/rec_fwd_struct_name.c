// Checks that the names we assign to implicit structs reflect binding rules.
//- @S defines StructS
struct S {
  struct R *rrr;
};
//- @R defines StructR
//- StructR named vname("R#c",_,_,_,_)
struct R;
