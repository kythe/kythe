// Tests explicit specialization of function templates with defns and decls.
//- @id defines/binding IdDecl
template <typename T> T id(T x);
//- @id defines/binding IdSpecDefn
template <> int id(int x) { return x; }
//- @id defines/binding IdDefn
template <typename T> T id(T x) { return T(); }
//- IdSpecDefn specializes TAppDefn
//- TAppDefn.node/kind tapp
//- TAppDefn param.0 IdDecl
