// Checks that calls to a function are routed to the declaration.
//- @a defines/binding FnADecl
//- FnADecl callableas CADecl
void a();
//- @b defines/binding FnB
//- AAnchor childof FnB
//- AAnchor ref/call CADecl
void b() { a(); }
//- @a defines/binding FnADefn
//- FnADefn callableas CADefn
void a() { }
//- !{ FnADefn callableas CADecl }
//- !{ FnADecl callableas CADefn }
