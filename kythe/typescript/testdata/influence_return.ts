/**
 * @fileoverview Test that functions are influenced by their return values.
 */

//- @fn defines/binding Function
function fn(a:number) {
  //- @a ref ParamA
  //- ParamA influences Function
  return a;
}
