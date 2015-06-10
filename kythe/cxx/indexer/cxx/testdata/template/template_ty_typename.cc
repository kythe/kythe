// Checks that typename template parameters are properly recorded.
template
//- @X defines AbsvX
<typename X>
//- @C defines Abs
class C {};
//- Abs param.0 AbsvX
//- AbsvX.node/kind absvar
//- AbsvX named vname("X#n",_,_,_,_)
