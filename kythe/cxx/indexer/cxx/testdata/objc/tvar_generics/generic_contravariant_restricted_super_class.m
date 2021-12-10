// Test that we handle restricted type arguments correctly when that
// restriction is based on a class.
//
// Also test that the contravariant type argument has the correct bounds.

@interface O
@end
@implementation O
@end

//- @ObjParent defines/binding ObjParentDecl
@interface ObjParent
@end

//- @ObjParent defines/binding ObjParentImpl
@implementation ObjParent
@end

//- @ObjChild defines/binding ObjChildDecl
@interface ObjChild : ObjParent
@end

//- @ObjChild defines/binding ObjChildImpl
@implementation ObjChild
@end

//- @ObjGrandChild defines/binding ObjGrandChildDecl
@interface ObjGrandChild : ObjChild
@end

//- @ObjGrandChild defines/binding ObjGrandChildImpl
@implementation ObjGrandChild
@end

// No source range defines BoxDecl since this is a generic type.
//- @Type defines/binding TypeVar
//- @Box defines/binding BoxDecl
//- TypeVar.node/kind tvar
//- BoxDecl tparam.0 TypeVar
//- TypeVar bounded/upper ObjParentDeclPtrType
//- ObjParentDeclPtrType.node/kind tapp
//- ObjParentDeclPtrType param.0 vname("ptr#builtin", _, _, _, _)
//- ObjParentDeclPtrType param.1 ObjParentImpl
//- @ObjParent ref ObjParentImpl
@interface Box<__contravariant Type: ObjParent*> : O
-(Type) compute;
@end

@implementation Box
-(id) compute {
  ObjChild *child = [[ObjChild alloc] init];
  return child;
}
@end

int main(int argc, char **argv) {
  // ObjChild* is a subtype of ObjParent*.
  Box<ObjChild *> *b1 = [[Box alloc] init];

  // Box<ObjChild *> is a subtype of Box<ObjGrandChild *> because we declared
  // the type argument to be contravariant.
  Box<ObjGrandChild *> *b2 = b1;

  return 0;
}
