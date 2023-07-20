// Test lightweight generics (invariant) used to define a class.
//
// This file also tests the implementation for a lightweight generic. Since the
// implementation knows nothing of the type arguments, it is treated like a
// regular implementation block. It is a child of the file, it completes the
// decl, etc.
//
// There is no actual test of the implied invariant attribute in this file. We
// don't record any information for invariance, so there is nothing to test
// beyond making sure the indexer doesn't crash.

@interface O
@end
@implementation O
@end

// Type parameter bounds cannot refer to other type parameters
// @interface Disallowed<T, X : T> : O
// @end

// No source range defines BoxDecl since this is a generic type.
//- @Type defines/binding TypeVar
//- @Box defines/binding BoxDecl
//- TypeVar.node/kind tvar
//- TypeVar.variance invariant
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
