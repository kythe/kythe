// Checks that Objective-C instance methods are declared and defined.

//- @Box defines/binding BoxIface
@interface Box

//- @foo defines/binding FooDecl
//- FooDecl.node/kind function
//- FooDecl.complete incomplete
//- FooDecl childof BoxIface
-(int) foo;

@end

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

