// Test that the indexer correctly handles compatibility_alias in the presence
// of a macro with parameters.
//
// TODO(salguarnieri) currently this test will fail because we do not handle
// this case correctly.

//- @Car defines/binding CarDecl
@interface Car
-(int) drive;
@end

//- @Car defines/binding CarImpl
@implementation Car
-(int) drive {
  return 20;
}
@end

#define Van(make, color) make ## VAN ## color

//- @"Van(Big, Red)" defines/binding VanAlias
//- @Car ref CarDecl
//- VanAlias aliases CarDecl
@compatibility_alias Van(Big, Red) Car;

int main(int argc, char **argv) {
  //- @van defines/binding VanVar
  //- VarVar typed PtrVanAlias
  BigVANRed *van = [[Van(Big, Red) alloc] init];
  return 0;
}
