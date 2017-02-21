// Test that subclasses extend and ref their superclass interfaces when the
// implementations are not present.

//- @Super defines/binding SuperDecl
@interface Super
//- @"foo" defines/binding FooDecl
-(int)foo;
@end

//- @Duper defines/binding DuperDecl
//- DuperDecl extends SuperDecl
//- @Super ref SuperDecl
@interface Duper : Super
-(int)foo;
-(int)bar;
@end

//- @Duper defines/binding DuperImpl
@implementation Duper
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
