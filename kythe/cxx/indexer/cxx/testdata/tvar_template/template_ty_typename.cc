// Checks that typename template parameters are properly recorded.
template
//- @X defines/binding AbsvX
<typename X>
//- @C defines/binding Abs
class C {};
//- Abs param.0 AbsvX
//- AbsvX.node/kind absvar
