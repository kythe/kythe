// Checks that simple, direct function calls are recorded.
//- @a defines FnA
//- FnA callableas CA
void a() { }
//- @b defines FnB
//- AAnchor childof FnB
//- AAnchor ref/call CA
void b() { a(); }
