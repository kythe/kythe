// Test that subclasses ref their superclass and overridden methods have the
// right edges when no implementations are provided.

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

//- @C2 defines/binding C2Interface
//- C2Interface extends SuperImpl
//- @Super ref SuperDecl
//- @Super ref SuperImpl
@interface C2 : Super
-(int)bar;
@end

int main(int argc, char** argv) {
  Super *s = [[Super alloc] init];
  Duper *d = [[Duper alloc] init];
  C2 *c2 = [[C2 alloc] init];
  Super *sd = d;

  //- @"[s foo]" ref/call FooImpl
  //- @foo ref FooImpl
  [s foo];
  //- @"[d foo]" ref/call FooDecl2
  //- @foo ref FooDecl2
  [d foo];
  //- @"[sd foo]" ref/call FooImpl
  //- @foo ref FooImpl
  [sd foo];
  //- @"[c2 foo]" ref/call FooDecl
  //- @foo ref FooDecl
  [c2 foo];
  return 0;
}
