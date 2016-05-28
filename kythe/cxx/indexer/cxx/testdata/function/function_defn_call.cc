// Checks that calls to a function are routed to the declaration.
//- @a defines/binding FnADecl
void a();
//- @b defines/binding FnB
//- AAnchor childof FnB
//- AAnchor ref/call FnADecl
void b() { a(); }
//- @a defines/binding FnADefn
void a() { }
