// We associate template total specialization documentation correctly.

/// Empty.
template <typename T> int S = 1;

//- @+2"/// Total." documents Var

/// Total.
template <> int S<int> = 2;

//- Var.node/kind variable
