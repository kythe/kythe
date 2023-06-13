// Checks that we associate template documentation correctly.
//- @+2"/// Empty." documents Record

/// Empty.
template <typename T> struct S { };

//- Record.node/kind record
