// Checks that variable references in a file are blamed on that file.

//- @x defines/binding VarX
int x;
//- @x ref VarX
//- @x childof FnInit
//- _FileInitializer defines/binding FnInit
//- FnInit.node/kind function
int y = x;
