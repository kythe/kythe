// Checks that Objective-C method calls provide links for the arguments.

//- @T defines/binding TDecl
@interface T
@end

//- @T defines/binding TImpl
@implementation T
@end

//- @Box defines/binding BoxIface
@interface Box

//- @foo defines/binding FooDecl
-(int) foo;

//- @bar defines/binding BarDecl
//- @k defines/binding KArgDecl
//- BarDecl param.0 KArgDecl
-(int) bar:(T*)k;

@end

//- @Box defines/binding BoxImpl
@implementation Box

//- @foo defines/binding FooDefn
//- FooDecl completedby FooDefn
-(int) foo {
  return 8;
}

//- @bar defines/binding BarDefn
//- BarDecl completedby BarDefn
//- @k defines/binding KArgDefn
//- BarDefn param.0 KArgDefn
-(int) bar:(T*) k {
  return 292;
}
@end

//- @main defines/binding Main
int main(int argc, char **argv) {
  //- @Box ref BoxImpl
  //- @box defines/binding BoxLocal
  Box *box;
  box = [[Box alloc] init];

  //- @"[box foo]" ref/call FooDefn
  //- @foo ref FooDefn
  //- @"[box foo]" childof Main
  //- @"foo" ref FooDefn
  //- @box ref BoxLocal
  [box foo];

  //- @tvar defines/binding TLocal
  T *tvar = [[T alloc] init];

  //- @"[box bar: tvar]" ref/call BarDefn
  //- @bar ref BarDefn
  //- @"[box bar: tvar]" childof Main
  //- @"bar" ref BarDefn
  //- @box ref BoxLocal
  //- @tvar ref TLocal
  [box bar: tvar];

  return 0;
}

