// Checks the representation of types from template instantiations.
template
<typename X>
struct C {
using T = typename X::Y;
};
struct D { using Y = int; };
using S = C<D>::T;
