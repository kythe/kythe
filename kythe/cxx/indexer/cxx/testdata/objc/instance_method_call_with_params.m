// Checks that Objective-C instance methods are called via the decl.

//- @Box defines/binding BoxIface
@interface Box

//- @"foo" defines/binding FooDecl
-(int) foo;

//- @"bar:(int)k" defines/binding BarDecl
-(int) bar:(int)k;

@end

//- @Box defines/binding BoxImpl
@implementation Box

//- @"foo " defines/binding FooDefn
//- @"foo " completes/uniquely FooDecl
-(int) foo {
  return 8;
}

//- @"bar:(int) k " defines/binding BarDefn
//- @"bar:(int) k " completes/uniquely BarDecl
-(int) bar:(int) k {
  return 28;
}
@end

//- @main defines/binding Main
int main(int argc, char **argv) {
  Box *box = [[Box alloc] init];

  //- @"[box foo]" ref/call FooDecl
  //- @"[box foo]" childof Main
  [box foo];

  //- @"[box bar: 38]" ref/call BarDecl
  //- @"[box bar: 38]" childof Main
  [box bar: 38];

  return 0;
}

