// Checks that function decls are recorded.
//- @f defines FDecl
void f();
//- FDecl.node/kind function
//- FDecl.complete incomplete
//- FDecl named FName
//- FDecl callableas FCallable
//- FCallable.node/kind callable