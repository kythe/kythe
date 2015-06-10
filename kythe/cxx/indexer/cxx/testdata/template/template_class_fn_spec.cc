// Checks that we get edges for member function specialization and calls
// thereto.
//- @F defines MemDecl
//- @C defines TmplDefn
template <typename T> struct C { static void F(); };

//- @F defines IntMemDefn
//- @F completes/uniquely IntMemDecl
template <> void C<int>::F() {}
//- IntMemDecl.complete incomplete
//- IntMemDefn.complete definition
//- IntMemDecl childof IntTmplSpec
//- IntMemDefn callableas IntMemCall
//- IntTmpl specializes TApp
//- TApp param.0 TmplDefn

//- @"C<int>::F()" ref/call IntMemCall
void dummy() { C<int>::F(); }
