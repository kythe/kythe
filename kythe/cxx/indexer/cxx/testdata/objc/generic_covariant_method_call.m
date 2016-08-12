// TODO(salguarnieri) Add assertions to make this a real test.

@interface O
@end

@implementation O
@end

@interface Box<__covariant Type>
-(int) addToList:(Type)item;
@end

@implementation Box
-(int) addToList:(id)item {
  return 1;
}
@end

int main(int argc, char **argv) {
  Box<O *> *b = [[Box alloc] init];
  O *o = [[O alloc] init];
  [b addToList:o];
  return 0;
}
