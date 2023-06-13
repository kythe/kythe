// Test instantiation of a lightweight generic class. The class decl should
// reference the Abs node, but the class instantiation should just treat this
// like a standard type application.

@interface Obj
@end

@implementation Obj
@end

//- @O defines/binding ODecl
@interface O
@end

//- @O defines/binding OImpl
@implementation O
@end

// No source range defines BoxDecl since this is a generic type.
//- @Type defines/binding TypeVar
//- @Box defines/binding BoxDecl
//- TypeVar.node/kind tvar
//- BoxDecl tparam.0 TypeVar
@interface Box<Type> : Obj
-(int) addToList:(Type)item;
@end

//- @Box defines/binding BoxImpl
@implementation Box
-(int) addToList:(id)item {
  return 1;
}
@end

int main(int argc, char **argv) {
  //- @O ref OImpl
  //- @box defines/binding BoxVar
  //- BoxVar typed PtrBoxVarType
  //- PtrBoxVarType.node/kind tapp
  //- PtrBoxVarType param.0 vname("ptr#builtin", _, _, _, _)
  //- PtrBoxVarType param.1 BoxVarType
  //- BoxVarType.node/kind tapp
  //- BoxVarType param.0 BoxImpl
  //- BoxVarType param.1 PtrOType
  //- PtrOType.node/kind tapp
  //- PtrOType param.0 vname("ptr#builtin", _, _, _, _)
  //- PtrOType param.1 OImpl
  Box<O *> *box = [[Box alloc] init];

  return 0;
}
