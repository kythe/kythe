//- @plus_one defines/binding FplusOne
fn plus_one() -> i32 {
  1
}
//- @foo defines/binding Ffoo
fn foo(){
  //- @"plus_one()" ref/call FplusOne
  //- @"plus_one()" childof Ffoo
  //- @plus_one ref FplusOne
  plus_one();
}
