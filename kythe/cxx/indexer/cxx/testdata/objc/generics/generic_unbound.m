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
//- @Box defines/binding BoxAbs
//- TypeVar.node/kind absvar
//- BoxDecl childof BoxAbs
//- BoxAbs.node/kind abs
//- BoxAbs param.0 TypeVar
@interface Box<Type> : O
-(int) doSomething:(Type)t;
@end

//- @Box defines/binding BoxImpl
//- File.node/kind file
//- @Box completes/uniquely BoxAbs
@implementation Box
-(int) doSomething:(id)t {
  return 0;
}
@end

int main(int argc, char **argv) {
  return 0;
}
