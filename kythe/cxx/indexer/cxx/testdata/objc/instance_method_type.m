// Checks that Objective-C method types are recorded correctly.

//- @Box defines/binding BoxIface
@interface Box

//- @foo defines/binding FooDecl
//- FooDecl.node/kind function
//- FooDecl typed FooTy
//- FooTy.node/kind FooTApp
//- FooTy param.0 vname("fn#builtin",_,_,_,_)
//- FooTy param.1 vname("int#builtin",_,_,_,_)
-(int) foo;

//- @bar defines/binding BarDecl
//- BarDecl.node/kind function
//- BarDecl typed BarTy
//- BarTy.node/kind BarTApp
//- BarTy param.0 vname("fn#builtin",_,_,_,_)
//- BarTy param.1 vname("int#builtin",_,_,_,_)
//- BarTy param.2 vname("float#builtin",_,_,_,_)
//- BarTy param.3 vname("int#builtin",_,_,_,_)
-(int) bar:(float)f withI:(int)i;

@end

//- @Box defines/binding BoxImpl
@implementation Box

//- @foo defines/binding FooDefn
//- FooDefn typed FooTy
-(int) foo {
  return 8;
}

//- @bar defines/binding BarDefn
//- BarDefn typed BarTy
-(int) bar:(float)f withI:(int)i {
  return ((int)f) * i;
}
@end

int main(int argc, char **argv) {
  return 0;
}

