// Test lightweight generics (contravariant) used to define a class. The
// contravariant modifier is ignored and should have no impact on the indexer.

// No source range defines BoxDecl since this is a generic type.
//- @Type defines/binding TypeVar
//- @Box defines/binding BoxDecl
//- TypeVar.node/kind tvar
//- TypeVar.variance contravariant
//- BoxDecl tparam.0 TypeVar
@interface Box<__contravariant Type>
-(int) doSomething:(Type)t;
@end

@implementation Box
-(int) doSomething:(id)t {
  return 0;
}
@end

int main(int argc, char **argv) {
  return 0;
}
