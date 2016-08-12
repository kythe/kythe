// todo(salguarnieri) add more verification.

//- @Box defines/binding BoxIface
@interface Box
@end

//- @Box defines/binding BoxImpl
@implementation Box
@end

//- @main defines/binding Main
int main(int argc, char **argv) {
  //- @box defines/binding BoxVar
  Box *box = [[Box alloc] init];

  return 0;
}

