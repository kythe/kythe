// Tests basic support for function template declarations and definitions.
template <typename T>
T
//- @id defines/binding Decl
id(T x);

template <typename T>
T
//- @id defines/binding Defn
//- @id completes/uniquely Decl
id(T x)
{ return x; }
//- Decl.node/kind function
//- Defn.node/kind function
//- Decl.complete incomplete
//- Defn.complete definition