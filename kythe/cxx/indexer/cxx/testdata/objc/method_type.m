// Test that methods have the correct type.


@interface Box

//- @foo defines/binding FooFuncVar
//- FooFuncVar typed FooFuncTy
//- FooFuncTy.node/kind TApp
//- FooFuncTy param.0 vname("fn#builtin", _, _, _, _)
//- FooFuncTy param.1 vname("int#builtin", _, _, _, _)
-(int) foo;

//- @bar defines/binding BarFuncVar
//- BarFuncVar typed BarFuncTy
//- BarFuncTy.node/kind TApp
//- BarFuncTy param.0 vname("fn#builtin", _, _, _, _)
//- BarFuncTy param.1 vname("int#builtin", _, _, _, _)
//- BarFuncTy param.2 vname("int#builtin", _, _, _, _)
-(int) bar:(int)i;

//- @bar defines/binding BarBazFuncVar
//- BarBazFuncVar typed BarBazFuncTy
//- BarBazFuncTy.node/kind TApp
//- BarBazFuncTy param.0 vname("fn#builtin", _, _, _, _)
//- BarBazFuncTy param.1 vname("int#builtin", _, _, _, _)
//- BarBazFuncTy param.2 vname("int#builtin", _, _, _, _)
//- BarBazFuncTy param.3 vname("char#builtin", _, _, _, _)
-(int) bar:(int)i baz:(char)c;

@end

@implementation Box

//- @foo defines/binding FooImpl
//- FooImpl typed FooFuncTy
-(int) foo {
  return 21;
}

//- @bar defines/binding BarImpl
//- BarImpl typed BarFuncTy
-(int) bar:(int)i {
  return 22;
}

//- @bar defines/binding BarBazImpl
//- BarBazImpl typed BarBazFuncTy
-(int) bar:(int)i baz:(char)c {
  return 23;
}

@end

int main(int argc, char **argv) {
  Box *box = [[Box alloc] init];

  [box foo];
  [box bar:22];
  [box bar:23 baz:'d'];

  return 0;
}
