// Test subclasses of generic types.

//- @O1 defines/binding O1Decl
@interface O1
@end

//- @O1 defines/binding O1Impl
@implementation O1
@end

//- O1Ptr.node/kind tapp
//- O1Ptr param.0 vname("ptr#builtin", _, _, _, _)
//- O1Ptr param.1 O1Impl

//- @O2 defines/binding O2Decl
@interface O2
@end

//- @O2 defines/binding O2Impl
@implementation O2
@end

// No source range defines BoxDecl since this is a generic type.
//- @Type defines/binding TypeVar
//- @Box defines/binding BoxAbs
//- @O1 ref O1Impl
//- TypeVar.node/kind absvar
//- TypeVar bounded/upper O1Ptr
//- BoxDecl childof BoxAbs
//- BoxAbs.node/kind abs
//- BoxAbs param.0 TypeVar
//- BoxDecl extends O2Impl
//- @O2 ref O2Impl
//- @O2 ref O2Decl
@interface Box<Type : O1*> : O2
@end

//- @Box defines/binding BoxImpl
@implementation Box
@end


// No source range defines PackageDecl since this is a generic type.
//- @Type defines/binding PTypeVar
//- @Package defines/binding PackageAbs
//- @O1 ref O1Impl
//- PTypeVar.node/kind absvar
//- PTypeVar bounded/upper O1Ptr
//- PackageDecl childof PackageAbs
//- PackageAbs.node/kind abs
//- PackageAbs param.0 PTypeVar
@interface Package<Type : O1*>
//- @Type ref PTypeVar
//- PackageDecl extends BoxType
//- @Box ref BoxImpl
//- BoxType.node/kind tapp
//- BoxType param.0 BoxImpl
//- BoxType param.1 PTypeVar
: Box<Type>
@end

@implementation Package
@end

int main(int argc, char **argv) {
  return 0;
}
