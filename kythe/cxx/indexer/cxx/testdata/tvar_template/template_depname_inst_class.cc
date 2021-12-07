// Checks the representation of resolved dependent names.
template
<template <typename> class T>
//- @C defines/binding AbsC
struct C {
using S = typename T<int>::D;
};
template
<typename Q>
//- @Z defines/binding AbsZ
struct Z {
using D = Q;
};
// Somewhere we need to close the loop wrt the substitutions here.
using U = C<Z>::S;
//- AbsC.node/kind abs
//- AbsZ.node/kind abs

