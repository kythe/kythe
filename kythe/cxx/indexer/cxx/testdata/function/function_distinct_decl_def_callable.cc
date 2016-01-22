// Decls and defs generate different callables.
//- @f defines/binding FnFDecl
//- FnFDecl callableas FnFDeclCallable
void f();
//- @f defines/binding FnFDefn
//- FnFDefn callableas FnFDefnCallable
void f() { }
//- !{ FnFDecl callableas FnFDefnCallable }
