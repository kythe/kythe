// Checks that function defns are recorded.
//- @f defines/binding FDefn
void f() { }
//- FDefn.node/kind function
//- FDefn.complete definition
//- FDefn named FName
//- FDefn callableas FCallable
//- FCallable.node/kind callable