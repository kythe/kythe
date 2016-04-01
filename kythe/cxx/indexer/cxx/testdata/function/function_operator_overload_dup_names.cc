// Checks that unique names are produced for implicit operators.
//- ImpA1 defines/binding OpAssignRvalueRef
//- ImpA1.subkind implicit
//- ImpA2 defines/binding OpAssignConstLvalueRef
//- ImpA2.subkind implicit
//- OpAssignConstLvalueRef param.0 ArgL0
//- ArgL0 typed TyL0
//- TyL0 param.0 vname("lvr#builtin",_,_,_,_)
//- OpAssignRvalueRef param.0 ArgR0
//- ArgR0 typed TyR0
//- TyR0 param.0 vname("rvr#builtin",_,_,_,_)
class A { };
void f() {
  A a1; a1 = A();
  a1 = a1;
}
