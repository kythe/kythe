// TODO(salguarnieri) Add assertions to make this a real test.

@interface ObjParent
@end

@implementation ObjParent
@end

@interface ObjChild : ObjParent
@end

@implementation ObjChild
@end

@interface Box<__covariant Type: ObjParent*>
-(int) addToList:(Type)item;
@end

@implementation Box
-(int) addToList:(id)item {
  return 1;
}
@end

int main(int argc, char **argv) {
  Box<ObjParent *> *b = [[Box alloc] init];
  ObjParent *o = [[ObjParent alloc] init];
  ObjChild *o2 = [[ObjChild alloc] init];
  [b addToList:o];
  [b addToList:o2];

  return 0;
}
