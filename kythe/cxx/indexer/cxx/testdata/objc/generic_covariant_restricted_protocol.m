// TODO(salguarnieri) Add assertions to make this a real test.

@protocol Proto
@end

@implementation ObjParent
@end

@interface ObjChild : ObjParent<Proto>
@end

@implementation ObjChild
@end


@interface Box<__covariant Type:id<Proto> >
-(int) addToList:(Type)item;
@end

@implementation Box
-(int) addToList:(id)item {
  return 1;
}
@end

int main(int argc, char **argv) {
  Box<ObjChild *> *b = [[Box alloc] init];
  ObjChild *o = [[ObjChild alloc] init];
  [b addToList:o];
  return 0;
}
