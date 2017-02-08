// Checks that Objective-C class methods ref/call the correct method.

//- @Box defines/binding BoxIface
@interface Box

//- @"foo" defines/binding FooDecl
//- FooDecl.node/kind function
//- FooDecl.complete incomplete
//- FooDecl childof BoxIface
+(int) foo;

@end

//- @Box defines/binding BoxImpl
@implementation Box

//- @"foo " defines/binding FooDefn
//- FooDefn.node/kind function
//- FooDefn.complete definition
//- FooDefn childof BoxImpl
//- @"foo " completes/uniquely FooDecl
+(int) foo {
  return 8;
}
@end

//- FooDecl named FooName
//- FooDefn named FooName

int main(int argc, char **argv) {
  //- @"[Box foo]" ref/call FooDefn
  //- @"[Box foo]" childof Main
  //- @"[Box foo]".node/kind anchor
  [Box foo];
  return 0;
}

