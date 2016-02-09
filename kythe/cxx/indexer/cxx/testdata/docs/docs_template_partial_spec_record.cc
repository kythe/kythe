// We associate template partial specialization documentation correctly.

template <typename T> struct S { };

//- @+3"/// Special." documents Abs
//- @+2"/// Special." documents Record

/// Special.
template <typename T> struct S<T*> { };

//- Abs.node/kind abs
//- Record.node/kind record
