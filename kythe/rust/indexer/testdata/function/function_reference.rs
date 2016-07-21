//- @plus_one defines/binding FplusOne
fn plus_one(a: &i32) -> i32 {
  a + 1
}
fn main(){
  //- @plus_one ref FplusOne
  [1,2,3].iter().map(plus_one);
}
