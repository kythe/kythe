// Checks that local variables are indexed.
//- @fn defines/binding Fn
function fn() {
//- @x defines/binding VarX
//- VarX named Name=vname("x#n",_,_,_,"ts")
//- VarX.node/kind variable
//- VarX childof Fn
  var x;
}
