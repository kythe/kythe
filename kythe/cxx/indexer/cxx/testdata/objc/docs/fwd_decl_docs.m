// Checks that forward declared classes don't record documentation.

//- @+4"Box" defines/binding BoxFwd
//- !{@+2"// Forward Decl" documents BoxFwd}

// Forward Decl
@class Box;

//- @+8"Box" defines/binding BoxIface
//- BoxIface.node/kind record
//- BoxIface.subkind class
//- BoxIface.complete complete

//- @+2"// This is a doc" documents BoxIface

// This is a doc
@interface Box
@end

//- @+9"Box" defines/binding BoxImpl
//- BoxImpl.node/kind record
//- BoxImpl.subkind class
//- BoxImpl.complete definition
//- BoxIface completedby BoxImpl

//- @+2"// This is a second doc" documents BoxImpl

// This is a second doc
@implementation Box
@end

int main(int argc, char **argv) {
  return 0;
}

