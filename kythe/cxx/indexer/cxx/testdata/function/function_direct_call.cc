// Checks that simple, direct function calls are recorded.
//- @a defines/binding FnA
//- FnA callableas CA
void a() { }
//- @b defines/binding FnB
//- AAnchor childof FnB
//- AAnchor ref/call CA
void b() { a(); }
