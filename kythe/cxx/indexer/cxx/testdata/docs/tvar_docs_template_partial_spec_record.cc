// We associate template partial specialization documentation correctly.

template <typename T> struct S { };

//- @+2"/// Special." documents Record

/// Special.
template <typename T> struct S<T*> { };

//- Record.node/kind record
