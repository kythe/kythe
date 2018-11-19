// We index USRs for records.

//- @C defines/binding C
//- CUsr /clang/usr C
//- CUsr.node/kind clang/usr
class C;

//- @U defines/binding U
//- UUsr /clang/usr U
//- UUsr.node/kind clang/usr
union U;

//- @S defines/binding S
//- SUsr /clang/usr S
//- SUsr.node/kind clang/usr
struct S {
  //- @m defines/binding M
  //- MUsr /clang/usr M
  //- MUsr.node/kind clang/usr
  int m;

  //- @f defines/binding F
  //- FUsr /clang/usr F
  //- FUsr.node/kind clang/usr
  void f() {
    //- @l defines/binding L
    //- !{_ /clang/usr L}
    int l;
  }
};
