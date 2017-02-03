// TODO(salguarnieri) Add assertions to make this a real test. This test is
// marked with the arc-ignore and manual tags in the BUILD file so it is
// not executed.

// TODO(salguarnieri) Locate this header file so this test can be indexed.
#import <objc/runtime.h>

@interface Box
-(int) foo;
-(int) bar;
@end

@implementation Box
+(void) load {
  // This is not entirely safe, in real code it would be in a dispatch once
  // block.
  Class me = [self class];
  SEL fooSel = @selector(foo);
  SEL barSel = @selector(bar);
  Method fooMethod = class_getInstanceMethod(me, fooSel);
  Method barMethod = class_getInstanceMethod(me, barSel);
  class_replaceMethod(
      me,
      fooSel,
      method_getImplementation(barMethod),
      method_getTypeEncoding(barMethod)
  );
}

-(int) foo {
  return 10;
}

-(int) bar {
  return 20;
}

@end

int main(int argc, char **argv) {
  Box *b = [[Box alloc] init];

  // fooval should be equal to barval due to the swizzle.
  int fooval = [b foo];
  int barval = [b bar];

  return 0;
}
