// Checks that the correct instance method is linked to each call site.
//
// The last method call also tests that our range for message expressions
// does not include any extra whitespace.

@interface Box
//- @"foo" defines/binding FooDecl
-(int) foo;

//- @"foo:(int)k" defines/binding FooKDecl
-(int) foo:(int)k;

//- @"foo:(int)k withBar:(int)bar" defines/binding FooKWithBarDecl
-(int) foo:(int)k withBar:(int)bar;
@end

@implementation Box
//- @"foo " defines/binding FooDefn
//- @"foo " completes/uniquely FooDecl
-(int) foo {
  return 8;
}

//- @"foo:(int)k " defines/binding FooKDefn
//- @"foo:(int)k " completes/uniquely FooKDecl
-(int) foo:(int)k {
  return k;
}

//- @"foo:(int)k withBar:(int)bar " defines/binding FooKWithBarDefn
//- @"foo:(int)k withBar:(int)bar " completes/uniquely FooKWithBarDecl
-(int) foo:(int)k withBar:(int)bar {
  return k * bar;
}
@end

//- @main defines/binding Main
int main(int argc, char **argv) {
  Box *box = [[Box alloc] init];

  //- @"[box foo]" childof Main
  //- @"[box foo]".node/kind anchor
  //- @"[box foo]" ref/call FooDecl
  [box foo];

  //- @"[box foo:30]" childof Main
  //- @"[box foo:30]".node/kind anchor
  //- @"[box foo:30]" ref/call FooKDecl
  [box foo:30];

  // Also test to make sure we don't get extra whitespace.
  //- @"[box foo:30 withBar:47]" childof Main
  //- @"[box foo:30 withBar:47]".node/kind anchor
  //- @"[box foo:30 withBar:47]" ref/call FooKWithBarDecl
  [box foo:30 withBar:47]    ;

  return 0;
}

