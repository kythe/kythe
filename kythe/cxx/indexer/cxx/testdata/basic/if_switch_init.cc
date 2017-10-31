// Verifies that init statements in if and switch index correctly.


struct S {
  //- @field defines/binding IntField
  int field;
};

S g();

void f() {
  //- @z defines/binding LocalVarZ
  if (auto z = g();
      //- @z ref LocalVarZ
      //- @field ref IntField
      z.field > 0) {
    //- @z ref LocalVarZ
    //- @field ref IntField
    int i = z.field;
  } else {
    //- @z ref LocalVarZ
    //- @field ref IntField
    int j = z.field;
  }

  int x;
  //- @zz defines/binding LocalVarZZ
  switch(auto zz = g();
         //- @zz ref LocalVarZZ
         //- @field ref IntField
         zz.field) {
    case 0:
    case 1:
      //- @zz ref LocalVarZZ
      //- @field ref IntField
      x = zz.field;
  }
}
