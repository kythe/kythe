// Checks that Objective-C method spans are correct when the method signature
// is broken up across several lines.

//- @Box defines/binding BoxIface
@interface Box

//- FooDeclAnchor.loc/start @^"foo"
//- FooDeclAnchor.loc/end @$+3"var2"
//- FooDeclAnchor defines/binding FooDecl
-(int) foo:(int) var1
       at:(int) var2;

-(int)
//- @"bar" defines/binding BarDecl
bar;

//- BazDeclAnchor.loc/start @^"baz"
//- BazDeclAnchor.loc/end @$+4"v3"
//- BazDeclAnchor defines/binding BazDecl
-(int) baz:(int) v1
       with:(int) v2
       without:(int)v3;

@end

//- @Box defines/binding BoxImpl
@implementation Box

//- FooDefnAnchor.loc/start @^"foo"
//- FooDefnAnchor.loc/end @$+4"var2 "
//- FooDefnAnchor defines/binding FooDefn
//- FooDefnAnchor completes/uniquely FooDecl
-(int) foo:(int) var1
       at:(int) var2 {
  return 8;
}

-(int)
//- @"bar " defines/binding BarDefn
//- @"bar " completes/uniquely BarDecl
bar {
  return 28;
}

//- BazDefnAnchor.loc/start @^"baz"
//- BazDefnAnchor.loc/end @$+5"v3 "
//- BazDefnAnchor defines/binding BazDefn
//- BazDefnAnchor completes/uniquely BazDecl
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
  //- FooCallAnchor.loc/end @$+5"at:10]"
  //- FooCallAnchor ref/call FooDefn
  //- FooCallAnchor childof Main
  //- FooCallAnchor.node/kind anchor
  [box foo:1
       at:10];

  //- @"[box bar]" ref/call BarDefn
  //- @"[box bar]" childof Main
  //- @"[box bar]".node/kind anchor
  [box bar];

  //- BazCallAnchor.loc/start @^"[box"
  //- BazCallAnchor.loc/end @$+7"]"
  //- BazCallAnchor ref/call BazDefn
  //- BazCallAnchor childof Main
  //- BazCallAnchor.node/kind anchor
  [box baz:20
       with:399
       without:1
       ];

  return 0;
}

