// Checks that we index function templates with mutliple forward declarations.
//- @f defines/binding Decl1
template <typename T> void f();
//- @f defines/binding Decl2
template <typename T> void f();
//- @f defines/binding Defn
//- @f completes/uniquely Decl1
//- @f completes/uniquely Decl2
template <typename T> void f() { }
