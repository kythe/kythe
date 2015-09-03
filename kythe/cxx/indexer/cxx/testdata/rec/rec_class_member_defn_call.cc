// Checks that calls within members are routed to the declaration.
//- @a defines/binding FnADecl
//- FnADecl callableas CADecl
void a();

struct S {
  //- @Inline defines/binding MemInline
  //- InlAAnchor childof MemInline
  //- InlAAnchor ref/call CADefn
  void Inline() { a(); }
  void External();
};

//- @External defines/binding MemExternal
//- ExtAAnchor childof MemExternal
//- ExtAAnchor ref/call CADefn
void S::External() { a(); }

//- @a defines/binding FnADefn
//- FnADefn callableas CADefn=CADecl
void a() {}
