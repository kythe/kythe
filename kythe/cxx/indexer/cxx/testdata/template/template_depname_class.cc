// Checks the representation of dependent names.
template
//- @T defines/binding DepT
<template <typename> class T>
struct C {
//- @D ref DepTIntD
using S = typename T<int>::D;
};
//- DepTIntD.text D
//- DepTIntD.node/kind lookup
//- DepTIntD param.0 DepTInt
//- DepTInt.node/kind tapp
//- DepTInt param.0 DepT
//- DepTInt param.1 Int
