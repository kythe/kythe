// Checks that calls within members are routed to the declaration.
//- @a defines FnADecl
//- FnADecl callableas CADecl
void a();

struct S {
  //- @Inline defines MemInline
  //- InlAAnchor childof MemInline
  //- InlAAnchor ref/call CADefn
  void Inline() { a(); }
  void External();
};

//- @External defines MemExternal
//- ExtAAnchor childof MemExternal
//- ExtAAnchor ref/call CADefn
void S::External() { a(); }

//- @a defines FnADefn
//- FnADefn callableas CADefn=CADecl
void a() {}
