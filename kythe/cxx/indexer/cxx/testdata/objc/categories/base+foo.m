// Test categories extend/category the base class and that methods declared in
// the category are correctly declared and defined.

#import "base+foo.h"
//- BaseFooDecl extends/category BaseDecl
//- BaseFooDecl.node/kind record
//- BaseFooDecl.subkind category
//- BaseFooDecl.complete complete

//- GetFooDecl.node/kind function
//- GetFooDecl.complete incomplete
//- GetFooDecl childof BaseFooDecl

//- @Base ref BaseDecl
//- @Foo defines/binding BaseFooImpl
//- BaseFooImpl extends/category BaseDecl
//- BaseFooImpl.node/kind record
//- BaseFooImpl.subkind category
//- BaseFooImpl.complete definition
//- BaseFooDecl completedby BaseFooImpl
@implementation Base (Foo)

//- @getFoo defines/binding GetFooImpl
//- GetFooDecl completedby GetFooImpl
//- GetFooImpl.node/kind function
//- GetFooImpl.complete definition
//- GetFooImpl childof BaseFooImpl
-(int) getFoo {
  return self.field * 20;
}

@end
