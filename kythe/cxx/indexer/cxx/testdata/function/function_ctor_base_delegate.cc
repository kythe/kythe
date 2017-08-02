//- @B defines/binding StructB
struct B {
  //- @B defines/binding BCtor
  B() {}
};

//- @D defines/binding StructD
//- @B ref StructB
struct D : B {
  //- @D defines/binding DCtor1
  //- @"B()" ref/call BCtor
  //- @B ref BCtor
  D(int): B() {}
  //- @D defines/binding DCtor0
  D():
    //- @"D(1)" ref/call DCtor1
    //- @D ref DCtor1
    D(1) {}
};
