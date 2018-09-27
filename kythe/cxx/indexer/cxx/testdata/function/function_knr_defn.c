// Verifies that functions with K&R style definitions are parsed correctly.

//- @fn defines/binding FnDefn
//- FnDefn param.0 ArgDefn
void fn(A)
//- @A defines/binding ArgDefn
  int A;
{ }
