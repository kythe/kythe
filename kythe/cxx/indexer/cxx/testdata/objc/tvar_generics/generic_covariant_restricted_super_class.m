// Test that we handle restricted type arguments correctly when that
// restriction is based on a class.
//
// Also test that the covariant type argument has the correct bounds.

@interface O
@end
@implementation O
@end

@interface Test<TTT> : O
@end
@implementation Test
@end

//- @ObjParent defines/binding ObjParentDecl
@interface ObjParent
@end

//- @ObjParent defines/binding ObjParentImpl
@implementation ObjParent
@end

@interface ObjChild : ObjParent
@end

@implementation ObjChild
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
@interface Box<__covariant Type: ObjParent*> : O
-(int) addToList:(Type)item;
@end

@implementation Box
-(int) addToList:(id)item {
  return 1;
}
@end

int main(int argc, char **argv) {
  Box<ObjChild *> *b1 = [[Box alloc] init];
  Box<ObjParent *> *b2 = b1;

  return 0;
}
