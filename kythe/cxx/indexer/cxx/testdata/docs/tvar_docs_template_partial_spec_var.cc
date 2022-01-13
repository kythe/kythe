// We associate template partial specialization documentation correctly.

/// Empty.
template <typename T> int S = 1;

//- @+3"/// Special." documents Abs
//- @+2"/// Special." documents Var

/// Special.
template <typename T> int S<T*> = 2;

//- Abs.node/kind abs
//- Var.node/kind variable
