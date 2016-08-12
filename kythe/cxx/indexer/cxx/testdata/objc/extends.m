// Test that subclasses ref their superclass and overridden methods have the
// right edges.

//- @Super defines/binding SuperInterface
@interface Super
//- @"foo" defines/binding FooDecl
-(int)foo;
@end

//- @Super defines/binding SuperImpl
@implementation Super
//- @"foo " defines/binding FooImpl
-(int)foo {
  return 200;
}
@end

//- @Duper defines/binding DuperInterface
//- DuperInterface extends SuperImpl
@interface Duper : Super
//- @"foo" defines/binding FooDecl2
//- FooDecl2 overrides FooDecl
-(int)foo;
-(int)bar;
@end

//- @Duper defines/binding DuperImpl
@implementation Duper
//- @"foo " defines/binding FooImpl2
//- FooImpl2 overrides FooDecl
-(int)foo {
  return 24;
}

-(int)bar {
  return 33;
}
@end

int main(int argc, char** argv) {
  return 0;
}
