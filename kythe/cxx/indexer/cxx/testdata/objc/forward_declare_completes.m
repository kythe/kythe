// Checks that forward declarations are completed correctly.

//- @Box defines/binding FwdDecl
//- FwdDecl.node/kind record
//- FwdDecl.complete incomplete
@class Box;

//- @Box defines/binding BoxIface
//- BoxIface.complete complete
//- @Box completes/uniquely FwdDecl
@interface Box

@end

//- @Box defines/binding BoxImpl
//- BoxImpl.complete definition
//- @Box completes/uniquely BoxIface
@implementation Box
@end

int main(int argc, char **argv) {
  return 0;
}

