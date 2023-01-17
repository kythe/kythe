// Checks that Objective-C instance methods are called via the decl.
//
// Also test that whitespace does not affect our source range for the message
// expression.

//- @Box defines/binding BoxIface
@interface Box

//- @foo defines/binding FooDecl
-(int) foo;
//- @bar defines/binding BarDecl
-(int) bar;

@end

//- @Box defines/binding BoxImpl
@implementation Box

//- @foo defines/binding FooDefn
//- FooDecl completedby FooDefn
-(int) foo {
  return 8;
}

//- @bar defines/binding BarDefn
//- BarDecl completedby BarDefn
-(int) bar {
  return 28;
}
@end

//- @main defines/binding Main
int main(int argc, char **argv) {
  Box *box = [[Box alloc] init];

  //- @"[box foo]" ref/call FooDefn
  //- @"[box foo]" childof Main
  //- @"[box foo]".node/kind anchor
  //- @foo ref FooDefn
  [box foo];

  //- @"[    box    bar    ]" ref/call BarDefn
  //- @"[    box    bar    ]" childof Main
  //- @"[    box    bar    ]".node/kind anchor
  //- @bar ref BarDefn
  [    box    bar    ]      ;

  return 0;
}

