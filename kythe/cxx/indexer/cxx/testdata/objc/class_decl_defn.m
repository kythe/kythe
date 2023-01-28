// Checks that Objective-C classes are declared and defined.

//- @Box defines/binding BoxIface
//- BoxIface.node/kind record
//- BoxIface.subkind class
//- BoxIface.complete complete
@interface Box
@end

//- @Box defines/binding BoxImpl
//- BoxImpl.node/kind record
//- BoxImpl.subkind class
//- BoxImpl.complete definition
//- BoxIface completedby BoxImpl
@implementation Box
@end

int main(int argc, char **argv) {
  return 0;
}

