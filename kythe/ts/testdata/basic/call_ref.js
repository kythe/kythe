// Checks that simple function calls are indexed.
//- @a defines/binding FnA
function a() { }
//- @b defines/binding FnB
//- AAnchor childof FnB
//- AAnchor ref/call FnA
function b() { a(); }
