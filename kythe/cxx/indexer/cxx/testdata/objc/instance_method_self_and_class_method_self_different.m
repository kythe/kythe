// todo(salguarnieri) This does not work yet
//
// Checks that self in a class method is different than self in an instance
// method.

//- @Box defines/binding BoxIface
@interface Box

-(int) foo;
-(int) bar;
+(int) baz;

@end

//- @Box defines/binding BoxImpl
@implementation Box

-(int) foo {
  //- @self ref InstanceSelf
  //- !{@self ref ClassSelf}
  return [self bar];
}

-(int) bar {
  return 28;
}

+(int) baz {
  //- @self ref ClassSelf
  //- !{@self ref InstanceSelf}
  Box *b = [[self alloc] init];
  [b foo];
  return 34;
}

@end

int main(int argc, char **argv) {
  return 0;
}

