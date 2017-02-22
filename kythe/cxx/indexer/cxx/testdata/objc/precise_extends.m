// Test that overridden methods only point back to the method they override
// and not all methods from the extended class (catches a bug that was in the
// code).

//- @Super defines/binding SuperInterface
@interface Super
//- @foo defines/binding FooDecl
-(int)foo;
//- @bar defines/binding BarDecl
-(int)bar;
@end

//- @Super defines/binding SuperImpl
@implementation Super
//- @foo defines/binding FooImpl
-(int)foo {
  return 200;
}
//- @bar defines/binding BarImpl
-(int)bar {
  return 202;
}
@end

//- @Duper defines/binding DuperInterface
//- DuperInterface extends SuperImpl
@interface Duper : Super
//- @foo defines/binding FooDecl2
//- FooDecl2 overrides FooDecl
//- !{FooDecl2 overrides BarDecl}
-(int)foo;
//- @bar defines/binding BarDecl2
//- BarDecl2 overrides BarDecl
//- !{BarDecl2 overrides FooDecl}
-(int)bar;
-(int)baz;
@end

//- @Duper defines/binding DuperImpl
@implementation Duper
//- @foo defines/binding FooImpl2
//- FooImpl2 overrides FooDecl
//- !{FooImpl2 overrides BarDecl}
-(int)foo {
  return 24;
}

//- @bar defines/binding BarImpl2
//- BarImpl2 overrides BarDecl
//- !{BarImpl2 overrides FooDecl}
-(int)bar {
  return 29;
}

-(int)baz {
  return 33;
}
@end

int main(int argc, char** argv) {
  return 0;
}
