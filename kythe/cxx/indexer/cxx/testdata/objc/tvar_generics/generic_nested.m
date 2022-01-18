// Test the source locations for nested type arguments and that the types
// are correct.

//- @O1 defines/binding O1Decl
@interface O1
@end

//- @O1 defines/binding O1Impl
@implementation O1
@end

//- @Fizzy defines/binding FizzyDecl
@interface Fizzy
@end

//- @Fizzy defines/binding FizzyImpl
@implementation Fizzy
@end

// No source range for CupDecl since this is a generic type.
@interface Cup<T> : O1
@end

//- @Cup defines/binding CupImpl
@implementation Cup
@end

// No source range for DrinkDecl sicne this is a generic type.
@interface Drink<T> : O1
@end

//- @Drink defines/binding DrinkImpl
@implementation Drink
@end

// No source range defines BoxDecl since this is a generic type.
//- @Type defines/binding TypeVar
//- @Box defines/binding BoxDecl
//- TypeVar.node/kind tvar
//- BoxDecl tparam.0 TypeVar
//- @Cup ref CupImpl
//- @Drink ref DrinkImpl
//- @Fizzy ref FizzyImpl
//- BoxDecl extends O1Impl
//- @O1 ref O1Decl
//- @O1 ref O1Impl
@interface Box<Type : Cup<Drink<Fizzy *>*>*> : O1
@end

//- TypeVar bounded/upper CupTypePtr
//- CupTypePtr.node/kind tapp
//- CupTypePtr param.0 vname("ptr#builtin", _, _, _, _)
//- CupTypePtr param.1 CupType
//- CupType.node/kind tapp
//- CupType param.0 CupImpl
//- CupType param.1 DrinkTypePtr
//- DrinkTypePtr.node/kind tapp
//- DrinkTypePtr param.0 vname("ptr#builtin", _, _, _, _)
//- DrinkTypePtr param.1 DrinkType
//- DrinkType.node/kind tapp
//- DrinkType param.0 DrinkImpl
//- DrinkType param.1 FizzyTypePtr
//- FizzyTypePtr.node/kind tapp
//- FizzyTypePtr param.0 vname("ptr#builtin", _, _, _, _)
//- FizzyTypePtr param.1 FizzyImpl

@implementation Box
@end

int main(int argc, char **argv) {
  // This is a trivial example since there are no other possible types to give
  // to Box.
  //- @box defines/binding BoxVar
  //- BoxVar typed BoxVarPtrType
  //- BoxVarPtrType.node/kind tapp
  //- BoxVarPtrType param.0 vname("ptr#builtin", _, _, _, _)
  //- BoxVarPtrType param.1 BoxType
  //- BoxType.node/kind tapp
  //- BoxType param.0 BoxImpl
  //- BoxType param.1 CupTypePtr
  Box<Cup<Drink<Fizzy *>*>*> *box = [[Box alloc] init];

  return 0;
}
