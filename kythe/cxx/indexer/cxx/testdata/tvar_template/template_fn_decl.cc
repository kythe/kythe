// Tests basic support for function template declarations.
template <typename T>
T
//- @id defines/binding IdDecl
id(T x);
//- IdDecl.node/kind function
//- IdDecl.complete incomplete
