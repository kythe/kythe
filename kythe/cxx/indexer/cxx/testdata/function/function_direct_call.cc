// Checks that simple, direct function calls are recorded.
//- @a defines/binding FnA
void a() { }
//- @b defines/binding FnB
//- AAnchor childof FnB
//- AAnchor ref/call FnA
void b() { a(); }
