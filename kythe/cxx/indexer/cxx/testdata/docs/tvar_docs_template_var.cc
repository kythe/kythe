// Checks that we associate template documentation correctly.
//- @+2"/// Empty." documents Var

/// Empty.
template <typename T> int V = 1;

//- Var.node/kind variable
