// todo(salguarnieri) Add assertions
// todo(salguarnieri) Add other versions of test that make use of clang
// auto-generating the synthesize.
@interface Box {
  int _width;
}
@property int width;
-(int) foo;
@end

@implementation Box
@synthesize width = _width;
-(int) foo {
  return 22;
}
@end

int main(int argc, char **argv) {
  Box *b = [[Box alloc] init];
  b.width = 20;
  [b foo];
  return 0;
}
