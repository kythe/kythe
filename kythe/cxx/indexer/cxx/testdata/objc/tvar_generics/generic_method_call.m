// Test a method call of a method that uses type arguments. This tests that
// the method refs are correct.

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

//- @Child defines/binding ChildDecl
@interface Child : O
@end

//- @Child defines/binding ChildImpl
@implementation Child
@end

// No source range defines BoxDecl since this is a generic type.
//- @Type defines/binding TypeVar
//- @Box defines/binding BoxDecl
//- TypeVar.node/kind tvar
//- BoxDecl tparam.0 TypeVar
@interface Box<Type> : Obj
//- @addToList defines/binding FuncDecl
-(int) addToList:(Type)item;
@end

//- @Box defines/binding BoxImpl
@implementation Box
//- @addToList defines/binding FuncImpl
-(int) addToList:(id)item {
  return 1;
}
@end

int main(int argc, char **argv) {
  Box<O *> *b = [[Box alloc] init];
  Child *child = [[Child alloc] init];

  //- @"[b addToList:child]" ref/call FuncImpl
  //- @addToList ref FuncImpl
  [b addToList:child];

  return 0;
}
