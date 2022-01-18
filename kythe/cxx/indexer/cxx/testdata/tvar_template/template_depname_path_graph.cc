// Checks that we index dependent name path graphs and that they reference
// the same nodes within a template, but not between them.
template
//- @T defines/binding CParamT
<template <typename> class T>
struct C {
//- @F ref CDepTIntDEF
//- CDepTIntDEF.text F
//- @E ref CDepTIntDE
//- CDepTIntDE.text E
//- @D ref CDepTIntD
//- CDepTIntD.text D
//- @T ref CParamT
using X = typename T<int>::D::E::F;

//- @F ref CDeptTIntDEF
//- @E ref CDeptTIntDE
//- @D ref CDeptTIntD
//- @T ref CParamT
using Y = typename T<int>::D::E::F;
};
//- CDeptTIntDEF param.0 CDeptTIntDE
//- CDeptTIntDE param.0 CDeptTIntD

template
//- @T defines/binding DParamT
<template <typename> class T>
struct D {
//- @F ref DDepTIntDEF
//- @E ref DDepTIntDE
//- @D ref DDepTIntD
//- @T ref DParamT
//- !{ @F ref CDepTIntDEF }
//- !{ @E ref CDepTIntDE }
//- !{ @D ref CDepTIntD }
//- !{ @T ref CParamT }
using Z = typename T<int>::D::E::F;
};
