// Tests that class constructor and destructor reference the class.

//- @C defines/binding CStruct
struct C {
  //- @C ref CStruct
  C();
  //- @C ref CStruct
  explicit C(int);
  //- @C ref CStruct
  ~C();
};

//- @#0C ref CStruct
//- @#1C ref CStruct
C::C() {}
