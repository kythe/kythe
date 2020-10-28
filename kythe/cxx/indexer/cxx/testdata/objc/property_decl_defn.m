// Test that properties are declared. Also check that the property usage refs
// the property decl. Also a basic check that a getter ref/calls the implicit
// getter decl.

//- @Box defines/binding BoxDecl
//- BoxDecl.node/kind record
//- BoxDecl.subkind class
//- BoxDecl.complete complete
@interface Box {

//- @"_testwidth" defines/binding WidthVarDecl
//- WidthVarDecl childof BoxDecl
  int _testwidth;
}

//- @width defines/binding WidthPropDecl
//- WidthPropDecl childof BoxDecl
@property int width;

@end

//- @Box defines/binding BoxImpl
//- BoxImpl.node/kind record
//- BoxImpl.subkind class
//- BoxImpl.complete definition
@implementation Box

// TODO(salguarnieri) Add assertion when we know what we want to represent in
// the graph
//#- @"_testwidth" ref WidthVarDecl
@synthesize width = _testwidth;

@end

int main(int argc, char **argv) {
  Box *box = [[Box alloc] init];

  //- @a defines/binding AVar
  //- @width ref WidthPropDecl
  //- @width ref/call GetterMethod
  int a = box.width;

  //- @a ref AVar
  int b = a;

  //- @"[box width]" ref/call GetterMethod
  int c = [box width];

  return 0;
}
