// Checks that calls within members are routed to the declaration.
//- @a defines/binding FnADecl
void a();

struct S {
  //- @Inline defines/binding MemInline
  //- InlAAnchor childof MemInline
  //- InlAAnchor ref/call FnADecl
  void Inline() { a(); }
  void External();
};

//- @External defines/binding MemExternal
//- ExtAAnchor childof MemExternal
//- ExtAAnchor ref/call FnADecl
void S::External() { a(); }

//- @a defines/binding FnADefn
//- !{@a defines/binding FnADecl}
void a() {}
