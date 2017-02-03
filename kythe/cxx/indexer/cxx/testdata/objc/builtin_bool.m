// Checks that using the builtin _Bool doesn't cause an error.

@interface Box
-(_Bool) foo;
@end

@implementation Box
-(_Bool) foo {
  return 0;
}
@end

int main(int argc, char **argv) {
  return 0;
}

