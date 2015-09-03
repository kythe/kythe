// Checks that the names we assign to implicit structs reflect binding rules.
//- @S defines/binding StructS
struct S {
  struct R *rrr;
};
//- @R defines/binding StructR
//- StructR named vname("R#c",_,_,_,_)
struct R;
