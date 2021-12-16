// Test the type of a method that uses type arguments.

// Let's get a handle on the id type
//- IdTy aliases IdPtrTy
//- IdPtrTy.node/kind tapp
//- IdPtrTy param.0 vname("ptr#builtin", _, _, _, _)
//- IdPtrTy param.1 vname("id#builtin", _, _, _, _)

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

//- @addToList defines/binding FuncDecl
//- FuncDecl typed FuncTy
//- FuncTy.node/kind TApp
//- FuncTy param.0 vname("fn#builtin", _, _, _, _)
//- FuncTy param.1 vname("int#builtin", _, _, _, _)
//- FuncTy param.2 TypeVar
-(int) addToList:(Type)item;

@end

//- @Box defines/binding BoxImpl
@implementation Box

//- @addToList defines/binding FuncImpl
//- FuncImpl typed FuncIdTy
//- FuncIdTy.node/kind TApp
//- FuncIdTy param.0 vname("fn#builtin", _, _, _, _)
//- FuncIdTy param.1 vname("int#builtin", _, _, _, _)
//- FuncIdTy param.2 IdTy
-(int) addToList:(id)item {
  return 1;
}

@end

int main(int argc, char **argv) {

  return 0;
}
