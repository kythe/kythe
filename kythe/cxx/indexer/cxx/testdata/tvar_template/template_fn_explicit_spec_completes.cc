// Checks that we get completion edges for explicit function template specs.
template <typename T> T id(T x);
//- @id defines/binding Decl1
template <> int id(int x);
//- @id defines/binding Defn
//- Decl1 completedby Defn
//- Decl2 completedby Defn
template <> int id(int x) { return x; }
//- @id defines/binding Decl2
template <> int id(int x);
