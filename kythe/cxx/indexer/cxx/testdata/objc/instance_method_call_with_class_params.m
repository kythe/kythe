// Checks that Objective-C method parameters are correctly linked to their types
// even when those types are non-trivial.

//- @T defines/binding TDecl
@interface T
-(int) baz;
@end

//- @T defines/binding TImpl
@implementation T
-(int) baz {
  return 281;
}
@end

//- @Box defines/binding BoxIface
@interface Box

//- @foo defines/binding FooDecl
-(int) foo;

//- @bar defines/binding BarDecl
//- @k defines/binding KArgDecl
//- BarDecl param.0 KArgDecl
//- KArgDecl typed TPtrTy
//- TPtrTy.node/kind tapp
//- TPtrTy param.0 vname("ptr#builtin",_,_,_,_)
//- TPtrTy param.1 TImpl
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
//- KArgDefn typed TPtrTy
-(int) bar:(T*) k {
  return [k baz];
}
@end

//- @main defines/binding Main
int main(int argc, char **argv) {
  Box *box = [[Box alloc] init];

  //- @"[box foo]" ref/call FooDefn
  //- @foo ref FooDefn
  //- @"[box foo]" childof Main
  [box foo];

  T *t = [[T alloc] init];

  //- @"[box bar: t]" ref/call BarDefn
  //- @bar ref BarDefn
  //- @"[box bar: t]" childof Main
  [box bar: t];

  return 0;
}

