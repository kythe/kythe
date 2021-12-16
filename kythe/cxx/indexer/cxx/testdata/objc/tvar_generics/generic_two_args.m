// Test multiple type arguments are referenced correctly.

@interface O
@end
@implementation O
@end

// No source range defines BoxDecl since this is a generic type.
//- @Box defines/binding BoxDecl
//- @FooType defines/binding FooTypeVar
//- @BarType defines/binding BarTypeVar
//- FooTypeVar.node/kind tvar
//- BarTypeVar.node/kind tvar
//- BoxDecl tparam.0 FooTypeVar
//- BoxDecl tparam.1 BarTypeVar
@interface Box<FooType, BarType> : O

//- @BarType ref BarTypeVar
//- @FooType ref FooTypeVar
-(BarType) doSomething:(FooType)t;

@end

@implementation Box
-(id) doSomething:(id)t {
  return 0;
}
@end

int main(int argc, char **argv) {
  return 0;
}
