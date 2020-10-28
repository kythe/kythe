//- @C defines/binding CDecl
struct C {
  //- @member defines/binding MemberDecl
  int member;
};

void fn() {
  //- @C ref CDecl
  //- @mptr defines/binding PtrDecl
  //- PtrDecl typed MemPtrType
  //- MemPtrType.node/kind tapp
  //- MemPtrType param.0 vname("mptr#builtin", "", "", "", "c++")
  //- MemPtrType param.1 vname("int#builtin", "", "", "", "c++")
  //- MemPtrType param.2 CDecl
  int C::* mptr;
  //- @C ref CDecl
  //- @member ref MemberDecl
  //- @mptr ref/writes PtrDecl
  mptr = &C::member;
}
