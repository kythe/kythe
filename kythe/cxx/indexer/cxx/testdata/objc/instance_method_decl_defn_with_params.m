// Checks that Objective-C instance methods with parameters are declared and
// defined.

//- @Box defines/binding BoxIface
@interface Box

//- @foo defines/binding FooDecl
//- FooDecl.node/kind function
//- FooDecl.complete incomplete
//- FooDecl childof BoxIface
-(int) foo;

//- @bar defines/binding BarDecl
//- BarDecl.node/kind function
//- BarDecl.complete incomplete
//- BarDecl childof BoxIface
-(int) bar:(int)k;

@end

//- @Box defines/binding BoxImpl
@implementation Box

//- @foo defines/binding FooDefn
//- FooDefn.node/kind function
//- FooDefn.complete definition
//- FooDefn childof BoxImpl
//- FooDecl completedby FooDefn
-(int) foo {
  return 8;
}

//- @bar defines/binding BarDefn
//- BarDefn.node/kind function
//- BarDefn.complete definition
//- BarDefn childof BoxImpl
//- BarDecl completedby BarDefn
-(int) bar:(int)k {
  return k * 2;
}
@end

int main(int argc, char **argv) {
  return 0;
}

