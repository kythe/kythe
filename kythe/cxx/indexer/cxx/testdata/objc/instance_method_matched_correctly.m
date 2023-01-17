// Checks that the correct instance method is linked to each call site.
//
// The last method call also tests that our range for message expressions
// does not include any extra whitespace.

@interface Box
//- @foo defines/binding FooDecl
-(int) foo;

//- @foo defines/binding FooKDecl
-(int) foo:(int)k;

//- @foo defines/binding FooKWithBarDecl
-(int) foo:(int)k withBar:(int)bar;
@end

@implementation Box
//- @foo defines/binding FooDefn
//- FooDecl completedby FooDefn
-(int) foo {
  return 8;
}

//- @foo defines/binding FooKDefn
//- FooKDecl completedby FooKDefn
-(int) foo:(int)k {
  return k;
}

//- @foo defines/binding FooKWithBarDefn
//- FooKWithBarDecl completedby FooKWithBarDefn
-(int) foo:(int)k withBar:(int)bar {
  return k * bar;
}
@end

//- @main defines/binding Main
int main(int argc, char **argv) {
  Box *box = [[Box alloc] init];

  //- @"[box foo]" childof Main
  //- @"[box foo]".node/kind anchor
  //- @"[box foo]" ref/call FooDefn
  //- @foo ref FooDefn
  [box foo];

  //- @"[box foo:30]" childof Main
  //- @"[box foo:30]".node/kind anchor
  //- @"[box foo:30]" ref/call FooKDefn
  //- @foo ref FooKDefn
  [box foo:30];

  // Also test to make sure we don't get extra whitespace.
  //- @"[box foo:30 withBar:47]" childof Main
  //- @"[box foo:30 withBar:47]".node/kind anchor
  //- @"[box foo:30 withBar:47]" ref/call FooKWithBarDefn
  //- @foo ref FooKWithBarDefn
  [box foo:30 withBar:47]    ;

  return 0;
}

