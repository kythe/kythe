// TODO(salguarnieri) Add assertions to make this a real test.

@interface Box<__covariant Type>
-(int) addToList:(Type)item;
@end

@implementation Box
-(int) addToList:(id)item {
  return 1;
}
@end

int main(int argc, char **argv) {
  return 0;
}
