// Test lightweight unbound generics used to define a class.

@interface O
@end
@implementation O
@end

// The generic type is unbound.
//- !{ TypeVar bounded/upper Anything0
//-    TypeVar bounded/lower Anything1 }

// No source range defines BoxDecl since this is a generic type.
//- @Type defines/binding TypeVar
//- @Box defines/binding BoxDecl
//- TypeVar.node/kind tvar
//- BoxDecl tparam.0 TypeVar
@interface Box<Type> : O
-(int) doSomething:(Type)t;
@end

//- @Box defines/binding BoxImpl
//- File.node/kind file
//- BoxDecl completedby BoxImpl
@implementation Box
-(int) doSomething:(id)t {
  return 0;
}
@end

int main(int argc, char **argv) {
  return 0;
}
