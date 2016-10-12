// This tests that block pointers have the expected type signatures. It also
// tests that typedefs of block pointers have the expected type signatures.
//
// Since it is common to leave out void if the function does not take any
// parameters, this test also tests the type returned in this "knr" style
// functions. This should already be tested in a cxx test, but it is here to
// show how it impacts blocks in Objective-C.

int main(int argc, char **argv) {
  //- @func defines/binding FuncVar
  //- FuncVar typed PtrFuncTy
  //- PtrFuncTy.node/kind TApp
  //- PtrFuncTy param.0 vname("ptr#builtin", _, _, _, _)
  //- PtrFuncTy param.1 FuncTy
  //- FuncTy.node/kind TApp
  //- FuncTy param.0 vname("fn#builtin", _, _, _, _)
  //- FuncTy param.1 vname("int#builtin", _, _, _, _)
  int (^func)(void) = ^(void) { return 120; };

  //- KNRTy.node/kind TApp
  //- KNRTy param.0 vname("ptr#builtin", _, _, _, _)
  //- KNRTy param.1 vname("knrfn#builtin", _, _, _, _)

  //- @funcNoVoid defines/binding FuncNoVoidVar
  //- FuncNoVoidVar typed KNRTy
  int (^funcNoVoid)() = ^() { return 120; };

  //- @funcVoidReturn defines/binding FuncVoidReturnVar
  //- FuncVoidReturnVar typed KNRTy
  void (^funcVoidReturn)() = ^() { return; };

  //- @funcAllVoid defines/binding FuncAllVoidVar
  //- FuncAllVoidVar typed PtrFuncAllVoidTy
  //- PtrFuncAllVoidTy.node/kind TApp
  //- PtrFuncAllVoidTy param.0 vname("ptr#builtin", _, _, _, _)
  //- PtrFuncAllVoidTy param.1 FuncAllVoidTy
  //- FuncAllVoidTy.node/kind TApp
  //- FuncAllVoidTy param.0 vname("fn#builtin", _, _, _, _)
  //- FuncAllVoidTy param.1 vname("void#builtin", _, _, _, _)
  void (^funcAllVoid)(void) = ^() { return; };

  //- @func2 defines/binding FuncVar2
  //- FuncVar2 typed PtrFunc2Ty
  //- PtrFunc2Ty.node/kind TApp
  //- PtrFunc2Ty param.0 vname("ptr#builtin", _, _, _, _)
  //- PtrFunc2Ty param.1 Func2Ty
  //- Func2Ty.node/kind TApp
  //- Func2Ty param.0 vname("fn#builtin", _, _, _, _)
  //- Func2Ty param.1 vname("short#builtin", _, _, _, _)
  //- Func2Ty param.2 vname("int#builtin", _, _, _, _)
  //- Func2Ty param.3 vname("long#builtin", _, _, _, _)
  short (^func2)(int, long) = ^(int i, long l) { return (short)120; };

  return 0;
}
