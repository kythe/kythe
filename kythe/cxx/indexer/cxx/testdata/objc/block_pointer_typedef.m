// This tests that we correctly parse block typedefs and assign the correct
// types to the typedef and variables that are defined using the typedef.

//- @Reducer defines/binding AliasReducer
//- AliasReducer aliases PtrReducerType
//- PtrReducerType.node/kind TApp
//- PtrReducerType param.0 vname("ptr#builtin", _, _, _, _)
//- PtrReducerType param.1 ReducerType
//- ReducerType.node/kind TApp
//- ReducerType param.0 vname("fn#builtin", _, _, _, _)
//- ReducerType param.1 vname("int#builtin", _, _, _, _)
//- ReducerType param.2 vname("int#builtin", _, _, _, _)
//- ReducerType param.3 vname("int#builtin", _, _, _, _)
typedef int (^Reducer)(int, int);

int main(int argc, char **argv) {
  //- @red defines/binding Reducer1
  //- Reducer1 typed AliasReducer
  Reducer red = ^(int a, int b) { return a * b; };

  //- @"red(10, 20)" ref/call Reducer1
  int b = red(10, 20);

  return b;
}
