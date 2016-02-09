// We attach documentation to function template specializations.

/// Empty.
template <typename T> void f(T t) { }

//- @+2"/// Special." documents Fun

/// Special.
template <> void f(int t) { }

//- Fun.node/kind function
