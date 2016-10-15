// Test to make sure the indexer doesn't crash when selectors are used. This
// *DOES NOT* test that selectors are linked correctly in the graph. They are
// a dynamic feature and even selectors that can be known through static
// analysis require non-trivial analysis. Furthermore, selectors that *are*
// known statically aren't really that useful or interesting.

@interface Num
@end

@implementation Num
@end

@interface Box
-(int) foo;

-(int) bar:(Num *)b;

-(int) bar:(Num *)bar withBaz:(Num *)baz;
@end

@implementation Box
-(int) foo {
  return 1;
}
-(int) bar:(int)b {
  return 2;
}
-(int) bar:(int)bar withBaz:(int)baz {
  return 3;
}
@end

int main(int argc, char **argv) {
  Num *n1 = [[Num alloc] init];
  Num *n2 = [[Num alloc] init];
  Box *box = [[Box alloc] init];

  //- @fooSel defines/binding FooSelVar
  SEL fooSel = @selector(foo);
  //- @fooSel ref FooSelVar
  [box performSelector:fooSel];

  //- @barSel defines/binding BarSelVar
  SEL barSel = @selector(bar:);
  //- @barSel ref BarSelVar
  [box performSelector:barSel withObject:n1];

  //- @barBazSel defines/binding BarBazSelVar
  SEL barBazSel = @selector(bar:withBaz:);
  //- @barBazSel ref BarBazSelVar
  [box performSelector:barBazSel withObject:n1 withObject:n2];

  [box performSelector:@selector(foo)];

  return 0;
}
