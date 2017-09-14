// Checks that we can generate a generates edge.
#pragma kythe_metadata "single_defn.meta"
//- vname(gsig, gcorp, groot, gpath, glang) generates VFoo
//- @foo defines/binding VFoo
int foo();
//- @foo defines/binding VFooDef
//- vname(gsig, gcorp, groot, gpath, glang) generates VFooDef
int foo() { }
