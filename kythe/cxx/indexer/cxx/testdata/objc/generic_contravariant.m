// TODO(salguarnieri) Add assertions to make this a real test.

@interface Box<__contravariant Type>
-(Type) doSomething;
@end

@implementation Box
-(id) doSomething {
  // nil == (id)0
  return (id)0;
}
@end

int main(int argc, char **argv) {
  return 0;
}
