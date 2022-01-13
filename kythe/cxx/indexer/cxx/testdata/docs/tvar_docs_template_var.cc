// Checks that we associate template documentation correctly.
//- @+3"/// Empty." documents Abs
//- @+2"/// Empty." documents Var

/// Empty.
template <typename T> int V = 1;

//- Abs.node/kind abs
//- Var.node/kind variable
