struct Num;
impl Num {
  //- @one defines/binding FOne
  fn one() {
  }
}

//- @main defines/binding Fmain
fn main(){
  //- @"Num::one()" ref/call FOne
  //- @"Num::one()" childof Fmain 
  //- @"Num::one" ref FOne
  Num::one();
}
