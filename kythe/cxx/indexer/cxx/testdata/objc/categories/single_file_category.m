//- @Base defines/binding BaseDecl
@interface Base {
  int _field;
}

@property int field;

-(instancetype) init;
-(int) baseFoo;
-(int) baseBar:(int) arg1 withF:(int) f;

@end


//- @Base defines/binding BaseImpl
@implementation Base

@synthesize field = _field;

-(instancetype)init {
  self->_field = 5;
  return self;
}

-(int) baseFoo {
  return _field;
}

-(int) baseBar:(int)arg1 withF:(int)f {
  return arg1 * f * _field;
}
@end

//- @Base ref BaseDecl
//- @Foo defines/binding BaseFooDecl
//- BaseFooDecl extends/category BaseDecl
//- BaseFooDecl.node/kind record
//- BaseFooDecl.subkind category
//- BaseFooDecl.complete complete
@interface Base (Foo)
  //- @getFoo defines/binding GetFooDecl
  //- GetFooDecl.node/kind function
  //- GetFooDecl.complete incomplete
  //- GetFooDecl childof BaseFooDecl
  -(int) getFoo;
@end



//- @"Base" ref BaseDecl
//- @"Foo" defines/binding BaseFooImpl
//- BaseFooImpl extends/category BaseDecl
//- BaseFooImpl.node/kind record
//- BaseFooImpl.subkind category
//- BaseFooImpl.complete definition
//- BaseFooDecl completedby BaseFooImpl
@implementation Base (Foo)

//- @"getFoo" defines/binding GetFooImpl
//- GetFooDecl completedby GetFooImpl
//- GetFooImpl.node/kind function
//- GetFooImpl.complete definition
//- GetFooImpl childof BaseFooImpl
-(int) getFoo {
  return self.field * 20;
}

@end
