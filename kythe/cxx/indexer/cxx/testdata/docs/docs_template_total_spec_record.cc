// We associate template total specialization documentation correctly.

/// Empty.
template <typename T> struct S { };

//- @+2"/// Total." documents Record

/// Total.
template <> struct S<int> { };

//- Record.node/kind record
