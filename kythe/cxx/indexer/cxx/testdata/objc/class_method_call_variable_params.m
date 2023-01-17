// Checks that Objective-C class method calls provide links for the arguments
// including the class receiver.

//- @Box defines/binding BoxIface
@interface Box

//- @foo defines/binding FooDecl
+(int) foo;

//- @bar defines/binding BarDecl
//- @k defines/binding KArgDecl
//- BarDecl param.0 KArgDecl
+(int) bar:(int)k;

@end

//- @Box defines/binding BoxImpl
@implementation Box

//- @foo defines/binding FooDefn
//- FooDecl completedby FooDefn
+(int) foo {
  return 8;
}

//- @bar defines/binding BarDefn
//- BarDecl completedby BarDefn
//- @k defines/binding KArgDefn
//- BarDefn param.0 KArgDefn
+(int) bar:(int) k {
  return k*2;
}
@end

//- @main defines/binding Main
int main(int argc, char **argv) {
  //- @"[Box foo]" ref/call FooDefn
  //- @"[Box foo]" childof Main
  //- @foo ref FooDefn
  //- @Box ref BoxImpl
  [Box foo];

  //- @tvar defines/binding TLocal
  int tvar = 109;


  //- @"[Box bar: tvar]" ref/call BarDefn
  //- @"[Box bar: tvar]" childof Main
  //- @bar ref BarDefn
  //- @tvar ref TLocal
  //- @Box ref BoxImpl
  [Box bar: tvar];

  return 0;
}

