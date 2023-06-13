// Checks that we get edges for member function specialization and calls
// thereto.
//- @F defines/binding _MemDecl
//- @C defines/binding TmplDefn
template <typename T> struct C { static void F(); };

//- @F defines/binding IntMemDefn
//- IntMemDecl completedby IntMemDefn
template <> void C<int>::F() {}
//- IntMemDecl.complete incomplete
//- IntMemDefn.complete definition
//- IntMemDefn.code _
//- IntMemDecl childof _IntTmplSpec
//- _IntTmpl specializes TApp
//- TApp param.0 TmplDefn

//- @"C<int>::F()" ref/call IntMemDefn
void dummy() { C<int>::F(); }
