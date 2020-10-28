// Tests that class constructor and destructor reference the class.

//- @C defines/binding CStruct
struct C {
  //- @C ref/id CStruct
  C();
  //- @C ref/id CStruct
  explicit C(int);
  //- @C ref/id CStruct
  ~C();
};

//- @#0C ref CStruct
//- @#1C ref/id CStruct
C::C() {}
