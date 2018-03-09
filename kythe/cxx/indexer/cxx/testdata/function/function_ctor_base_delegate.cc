namespace ns {

//- @B defines/binding StructB
struct B {
  //- @B defines/binding BCtor
  B(int = 0, void* = nullptr) {}
};

}  // namespace ns

//- @C defines/binding StructC
//- @B ref StructB
struct C : ns::B {
  //- @C defines/binding CCtor0
  //- @"::ns::B()" ref/call BCtor
  //- @B ref BCtor
  C(): ::ns::B() {}
  //- @C defines/binding CCtor1
  //- @"::ns::B(0)" ref/call BCtor
  //- @B ref BCtor
  C(int): ::ns::B(0) {}
  //- @C defines/binding CCtor2
  //- @"::ns::B(1, nullptr)" ref/call BCtor
  //- @B ref BCtor
  C(int, void*): ::ns::B(1, nullptr) {}
};

//- @D defines/binding StructD
//- @C ref StructC
struct D : C {
  //- @#0D defines/binding DCtor0
  //- @"D(1)" ref/call DCtor1
  //- @#1D ref DCtor1
  D(): D(1) {}
  //- @D defines/binding DCtor1
  //- @"C()" ref/call CCtor0
  //- @C ref CCtor0
  D(int): C() {}
  //- @D defines/binding DCtor2
  //- @"C(0)" ref/call CCtor1
  //- @C ref CCtor1
  D(char): C(0) {}
  //- @D defines/binding DCtor3
  //- @"C(1, nullptr)" ref/call CCtor2
  //- @C ref CCtor2
  D(void*): C(1, nullptr) {}
};
