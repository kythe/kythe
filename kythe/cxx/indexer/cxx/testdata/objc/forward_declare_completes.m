// Checks that forward declarations are completed correctly.

//- @Box defines/binding FwdDecl
//- FwdDecl.node/kind record
//- FwdDecl.complete incomplete
@class Box;

//- @Box defines/binding BoxIface
//- BoxIface.complete complete
//- FwdDecl completedby BoxIface
@interface Box

@end

//- @Box defines/binding BoxImpl
//- BoxImpl.complete definition
//- BoxIface completedby BoxImpl
@implementation Box
@end

int main(int argc, char **argv) {
  return 0;
}

