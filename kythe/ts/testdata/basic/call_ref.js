// Checks that simple function calls are indexed.
//- @a defines/binding FnA
//- FnA callableas CA
function a() { }
//- @b defines/binding FnB
//- AAnchor childof FnB
//- AAnchor ref/call CA
function b() { a(); }
