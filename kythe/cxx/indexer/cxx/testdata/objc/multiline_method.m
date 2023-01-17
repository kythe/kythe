// Checks that Objective-C method spans are correct when the method signature
// is broken up across several lines.

//- @Box defines/binding BoxIface
@interface Box

//- @foo defines/binding FooDecl
-(int) foo:(int) var1
       at:(int) var2;

-(int)
//- @bar defines/binding BarDecl
bar;

//- @baz defines/binding BazDecl
-(int) baz:(int) v1
       with:(int) v2
       without:(int)v3;

@end

//- @Box defines/binding BoxImpl
@implementation Box

//- @foo defines/binding FooDefn
//- FooDecl completedby FooDefn
-(int) foo:(int) var1
       at:(int) var2 {
  return 8;
}

-(int)
//- @bar defines/binding BarDefn
//- BarDecl completedby BarDefn
bar {
  return 28;
}

//- @baz defines/binding BazDefn
//- BazDecl completedby BazDefn
-(int) baz:(int) v1
       with:(int) v2
       without:(int)v3 {
  return 2;
}
@end

//- @main defines/binding Main
int main(int argc, char **argv) {
  Box *box = [[Box alloc] init];

  //- FooCallAnchor.loc/start @^"[box"
  //- FooCallAnchor.loc/end @$+6"at:10]"
  //- FooCallAnchor ref/call FooDefn
  //- FooCallAnchor childof Main
  //- FooCallAnchor.node/kind anchor
  //- @foo ref FooDefn
  [box foo:1
       at:10];

  //- @"[box bar]" ref/call BarDefn
  //- @"[box bar]" childof Main
  //- @"[box bar]".node/kind anchor
  //- @bar ref BarDefn
  [box bar];


  //- BazCallAnchor.loc/start @^"[box"
  //- BazCallAnchor.loc/end @$+8"]"
  //- BazCallAnchor ref/call BazDefn
  //- BazCallAnchor childof Main
  //- BazCallAnchor.node/kind anchor
  //- @baz ref BazDefn
  [box baz:20
       with:399
       without:1
       ];

  return 0;
}

