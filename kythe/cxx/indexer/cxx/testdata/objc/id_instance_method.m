// Ensure that methods called on objects of id type point back to a specific
// decl if possible.
//
// Also tests that whitespace before the selector doesn't show up in our
// ranges. The range for "-(int)     foo  ;" should just be "foo".

//- @Box defines/binding BoxIface
@interface Box

-(id) ident;

// todo(salguarnieri) For now, we have too much whitespace.
// Test that whitespace gets removed for the source range.
//- @foo defines/binding FooDecl
-(int)    foo   ;

@end

//- @Box defines/binding BoxImpl
@implementation Box

-(id) ident {
  return self;
}

//- @foo defines/binding FooDefn
-(int) foo {
  return 20;
}

@end

//- @main defines/binding Main
int main(int argc, char **argv) {
  Box *b = [[Box alloc] init];
  id bClone = [b ident];

  //- @"[bClone foo]" childof Main
  //- @"[bClone foo]".node/kind anchor
  //- @"[bClone foo]" ref/call FooDecl
  //- @foo ref FooDecl
  int s = [bClone foo];

  return 0;
}
