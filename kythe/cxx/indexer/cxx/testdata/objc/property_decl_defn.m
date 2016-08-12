// todo(salguarnieri) add assertions

@interface Box {
  int _width;
}
@property int width;
@end

@implementation Box
@synthesize width = _width;
@end

int main(int argc, char **argv) {
  return 0;
}
