// Checks that calls to a function are routed to the declaration.
//- @a defines FnADecl
//- FnADecl callableas CADecl
void a();
//- @b defines FnB
//- AAnchor childof FnB
//- AAnchor ref/call CADefn
void b() { a(); }
//- @a defines FnADefn
//- FnADefn callableas CADefn=CADecl
void a() { }
