// Tests basic support for function template declarations.
template <typename T>
T
//- @id defines/binding Abs
id(T x);
//- Abs.node/kind abs
//- IdDecl childof Abs
//- IdDecl.node/kind function
//- IdDecl.complete incomplete
