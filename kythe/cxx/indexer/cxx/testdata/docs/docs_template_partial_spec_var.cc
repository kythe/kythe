// We associate template partial specialization documentation correctly.

/// Empty.
template <typename T> int S = 1;

//- @+2"/// Special." documents Var

/// Special.
template <typename T> int S<T*> = 2;

//- Var.node/kind variable
