// Checks that we index dependent name path graphs.
template
<template <typename> class T>
struct C {
//- @F ref DepTIntDEF
//- DepTIntDEF.text F
//- @E ref DepTIntDE
//- DepTIntDE.text E
//- @D ref DepTIntD
//- DepTIntD.text D
using S = typename T<int>::D::E::F;
};
//- DepTIntDEF param.0 DepTIntDE
//- DepTIntDE param.0 DepTIntD
