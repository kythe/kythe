// Tests that declarations of template specializations are recorded.
//- @id defines/binding IdDecl
template <typename T> T id(T x);
//- @id defines/binding IdSpecDecl
//- IdSpecDecl specializes TApp
//- TApp param.0 IdDecl
template <> int id(int x);
//- @id defines/binding IdDecl2
template <typename T> T id(T x);
//- @id defines/binding IdSpecDecl2
//- IdSpecDecl2 specializes TApp2
//- TApp2 param.0 IdDecl
template <> int id(int x);
