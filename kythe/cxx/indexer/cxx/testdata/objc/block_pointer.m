// This tests that block pointers produce the correct defines/binding and
// ref/call edges.
//
// There isn't really anything that the Objective-C indexer needs to do. The
// AST visitor will visit the parts of the block nodes and the block itself is
// basically just a C function pointer.

int main(int argc, char **argv) {
  // This is a VarDecl.
  //- @a1 defines/binding A1FuncDecl
  int (^a1)();

  // This is a BlockDecl and a BlockExpr.
  a1 = ^() { return 200; };

  // This is a CallExpr.
  //- @"a1()" ref/call A1FuncDecl
  int a = a1();

  // This is a VarDecl, BlockDecl, and BlockExpr.
  //- @func defines/binding Func
  int (^func)() = ^() { return 120; };

  // This is a CallExpr.
  //- @"func()" ref/call Func
  int b = func();

  return b;
}
