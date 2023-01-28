// Test that a forward declared class declared before the @interface does not
// complete anything.


//- @FwdClass defines/binding FwdDecl
//- FwdDecl.node/kind record
@class FwdClass;

// FwdDecl completedby FwdIface=vname(_,_,_,"kythe/cxx/indexer/cxx/testdata/objc/FwdClass.h",_)
// FwdIface.node/kind anchor

#import "FwdClass.h"

//- @Box defines/binding BoxIface
@interface Box

//- @foo defines/binding FooDecl
//- @p1 defines/binding P1ArgDecl
//- FooDecl param.0 P1ArgDecl
//- P1ArgDecl typed P1PtrTy
//- P1PtrTy.node/kind tapp
//- P1PtrTy param.0 vname("ptr#builtin",_,_,_,_)
//- P1PtrTy param.1 FwdClassType
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

