// Checks that we index function templates with mutliple forward declarations.
//- @f defines/binding Decl1
template <typename T> void f();
//- @f defines/binding Decl2
template <typename T> void f();
//- @f defines/binding Defn
//- Decl1 completedby Defn
//- Decl2 completedby Defn
template <typename T> void f() { }
