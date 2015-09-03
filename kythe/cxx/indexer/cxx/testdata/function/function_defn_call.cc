// Checks that calls to a function are routed to the declaration.
//- @a defines/binding FnADecl
//- FnADecl callableas CADecl
void a();
//- @b defines/binding FnB
//- AAnchor childof FnB
//- AAnchor ref/call CADefn
void b() { a(); }
//- @a defines/binding FnADefn
//- FnADefn callableas CADefn=CADecl
void a() { }
