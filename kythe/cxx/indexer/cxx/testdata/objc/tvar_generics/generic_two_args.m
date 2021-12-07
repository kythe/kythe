// Test multiple type arguments are referenced correctly.

@interface O
@end
@implementation O
@end

// No source range defines BoxDecl since this is a generic type.
//- @Box defines/binding BoxAbs
//- BoxDecl childof BoxAbs
//- @FooType defines/binding FooTypeVar
//- @BarType defines/binding BarTypeVar
//- FooTypeVar.node/kind absvar
//- BarTypeVar.node/kind absvar
//- BoxAbs.node/kind abs
//- BoxAbs param.0 FooTypeVar
//- BoxAbs param.1 BarTypeVar
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
