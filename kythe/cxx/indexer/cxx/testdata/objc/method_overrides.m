// Test that subclasses ref their superclass and overridden methods have the
// right edges.

//- @Super defines/binding SuperDecl
@interface Super
//- @foo defines/binding FooDecl
-(int)foo;
@end

//- @Super defines/binding SuperImpl
@implementation Super
//- @foo defines/binding FooImpl
-(int)foo {
  return 200;
}
@end

//- @Duper defines/binding DuperInterface
//- DuperInterface extends SuperImpl
@interface Duper : Super
//- @foo defines/binding FooDecl2
//- FooDecl2 overrides FooDecl
-(int)foo;
-(int)bar;
@end

//- @Duper defines/binding DuperImpl
@implementation Duper
//- @foo defines/binding FooImpl2
//- FooImpl2 overrides FooDecl
-(int)foo {
  return 24;
}

-(int)bar {
  return 33;
}
@end

//- @C2 defines/binding C2Interface
//- C2Interface extends SuperImpl
//- @Super ref SuperDecl
//- @Super ref SuperImpl
@interface C2 : Super
-(int)bar;
@end

//- @C2 defines/binding C2Impl
@implementation C2
//- @foo defines/binding FooImplC2
//- FooImplC2 overrides FooDecl
-(int)foo {
  return 24;
}

-(int)bar {
  return 33;
}
@end

int main(int argc, char** argv) {
  Super *s = [[Super alloc] init];
  Duper *d = [[Duper alloc] init];
  C2 *c2 = [[C2 alloc] init];
  Super *sd = d;

  //- @"[s foo]" ref/call FooImpl
  //- @foo ref FooImpl
  [s foo];
  //- @"[d foo]" ref/call FooImpl2
  //- @foo ref FooImpl2
  [d foo];
  //- @"[sd foo]" ref/call FooImpl
  //- @foo ref FooImpl
  [sd foo];
  //- @"[c2 foo]" ref/call FooImplC2
  //- @foo ref FooImplC2
  [c2 foo];
  return 0;
}
