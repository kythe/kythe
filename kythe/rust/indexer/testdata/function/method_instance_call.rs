struct Num;
impl Num {
  //- @one defines/binding FOne
  fn one(&self) -> Num {
    Num
  }

  //- @two defines/binding FTwo
  fn two(&self) {
    ()
  }
}

//- @main defines/binding Fmain
fn main(){
  let a = Num;

  //- @"a.one()" ref/call FOne
  //- @"a.one()" childof Fmain
  //- @one ref FOne
  //- @"a.one().two()" ref/call FTwo
  //- @"a.one().two()" childof Fmain
  //- @two ref FTwo
  a.one().two();
}

