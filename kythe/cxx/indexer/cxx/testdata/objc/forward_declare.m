// Checks that forward declarations are referenced correctly.

//- @FwdClass defines/binding FwdDecl
//- FwdDecl.node/kind record
//- FwdDecl.complete incomplete
@class FwdClass;

//- @Box defines/binding BoxIface
@interface Box

// Since we won't have the impl of FwdClass, we have to refer to it by name in
// our tapp below.
//
//- @foo defines/binding FooDecl
//- @p1 defines/binding P1ArgDecl
//- FooDecl param.0 P1ArgDecl
//- P1ArgDecl typed P1PtrTy
//- P1PtrTy.node/kind tapp
//- P1PtrTy param.0 vname("ptr#builtin",_,_,_,_)
//- P1PtrTy param.1 FwdClassType
//- FwdClassType.node/kind tnominal
-(int) foo:(FwdClass*)p1;

@end

//- @Box defines/binding BoxImpl
@implementation Box
-(int) foo:(FwdClass*)p1 {
  return 10;
}
@end

int main(int argc, char **argv) {
  return 0;
}

