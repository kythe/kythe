// Checks that Objective-C instance methods are declared and defined when
// using a .h file.

#import "include.h"
//- FooDecl.node/kind function
//- FooDecl.complete incomplete
//- FooDecl childof BoxIface

//- @Box defines/binding BoxImpl
@implementation Box

//- @foo defines/binding FooDefn
//- FooDecl completedby FooDefn
//- FooDefn.node/kind function
//- FooDefn.complete definition
//- FooDefn childof BoxImpl
-(int) foo {
  return 8;
}

@end

int main(int argc, char **argv) {
  return 0;
}

