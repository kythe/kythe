// We attach documentation to function templates.

//- @+3"/// Empty." documents Fun
//- @+2"/// Empty." documents Abs

/// Empty.
template <typename T> void f(T t) { }

//- Fun.node/kind function
//- Abs.node/kind abs
