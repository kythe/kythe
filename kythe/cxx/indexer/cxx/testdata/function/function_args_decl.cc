// Checks that arguments to function declarations are correctly recorded.
//- @F defines/binding FnF
//- @A defines/binding ArgA
//- FnF param.0 ArgA
//- FnF param.1 Arg1
//- Arg1 typed ShortTy
//- ArgA typed FloatTy
//- Arg1.complete incomplete
//- ArgA.complete incomplete
void F(float A, short);
