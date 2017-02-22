// Test that subclasses extend and ref their superclass. Test that overridden
// methods have the right edges.

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

// For the reason why @Super ref SuperDecl and SuperImpl see
// IndexerASTVisitor::ConnectToSuperClassAndProtocols where we generate the
// node for the superclass.
//
//- @Duper defines/binding DuperDecl
//- DuperDecl extends SuperImpl
//- @Super ref SuperDecl
//- @Super ref SuperImpl
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

int main(int argc, char** argv) {
  return 0;
}
