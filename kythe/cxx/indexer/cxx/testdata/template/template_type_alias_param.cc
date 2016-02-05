// Checks that we don't fall over on type alias type parameters.

template <typename T>
struct tstruct {};

template <typename T>
using alias = typename tstruct<T>::type;
