// Checks that we associate template documentation correctly.
//- @+3"/// Empty." documents Abs
//- @+2"/// Empty." documents Record

/// Empty.
template <typename T> struct S { };

//- Abs.node/kind abs
//- Record.node/kind record