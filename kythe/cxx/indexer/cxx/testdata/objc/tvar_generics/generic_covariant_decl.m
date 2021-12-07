// Test lightweight generics (covariant) used to define a class. The covariant
// modifier is ignored and should have no impact on the indexer.

// No source range defines BoxDecl since this is a generic type.
//- @Type defines/binding TypeVar
//- @Box defines/binding BoxAbs
//- TypeVar.node/kind absvar
//- TypeVar.variance covariant
//- BoxDecl childof BoxAbs
//- BoxAbs.node/kind abs
//- BoxAbs param.0 TypeVar
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
