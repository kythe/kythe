// Test lightweight bound generics used to define a class.

//- @O1 defines/binding O1Decl
@interface O1
@end

//- @O1 defines/binding O1Impl
@implementation O1
@end

//- @O2 defines/binding O2Decl
@interface O2
@end

//- @O2 defines/binding O2Impl
@implementation O2
@end

// No source range defines BoxDecl since this is a generic type.
//- @Type defines/binding TypeVar
//- @Box defines/binding BoxDecl
//- TypeVar.node/kind tvar
//- BoxDecl tparam.0 TypeVar
//- TypeVar bounded/upper O2Ptr
//- O2Ptr.node/kind tapp
//- O2Ptr param.0 vname("ptr#builtin", _, _, _, _)
//- O2Ptr param.1 O2Impl
//- @O2 ref O2Impl
@interface Box<Type:O2*> : O1
-(int) doSomething:(Type)t;
@end

//- BoxDecl completedby BoxImpl
//- @Box defines/binding BoxImpl
//- File.node/kind file
@implementation Box
-(int) doSomething:(id)t {
  return 0;
}
@end

int main(int argc, char **argv) {
  return 0;
}
