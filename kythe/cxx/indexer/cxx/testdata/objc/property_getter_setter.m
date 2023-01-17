// Test calling the default getter and a semi-custom setter.
//
// The getter has the default name and the default implementation. The setter
// has the default name but a custom implementation.
//
// The test assures that:
// * Regular method calls to the setter and getter methods ref/call
//   the implicit decl.
// * Setting and getting via the property ref the property decl and ref/call
//   the implicit decl.
// * A custom impl of the setter properly completes the implicit decl.

//- @Box defines/binding BoxDecl
//- BoxDecl.node/kind record
//- BoxDecl.subkind class
//- BoxDecl.complete complete
@interface Box {

//- @"_testwidth" defines/binding WidthVarDecl
  int _testwidth;
}

//- @width defines/binding WidthPropDecl
@property int width;

@end

//- @Box defines/binding BoxImpl
//- BoxImpl.node/kind record
//- BoxImpl.subkind class
//- BoxImpl.complete definition
@implementation Box

@synthesize width = _testwidth;

//- @setWidth defines/binding SetWidthDefn
//- SetWidthDefn.node/kind function
//- SetWidthDefn childof BoxImpl
//- SetWidthDecl completedby SetWidthDefn
-(void) setWidth:(int)value {
  self->_testwidth = value;
}

@end

//- @main defines/binding Main
int main(int argc, char **argv) {
  Box *box = [[Box alloc] init];

  //- @width ref/call SetWidthDecl
  //- @width ref WidthPropDecl
  //- @width childof Main
  //- @width.node/kind anchor
  box.width = 44;

  //- @"[box setWidth:55]" childof Main
  //- @"[box setWidth:55]".node/kind anchor
  //- @"[box setWidth:55]" ref/call SetWidthDefn
  //- @setWidth ref SetWidthDefn
  [box setWidth:55];

  //- @width ref/call GetWidthMethod
  //- @width ref WidthPropDecl
  //- @width childof Main
  //- @width.node/kind anchor
  int a = box.width;

  //- @"[box width]" ref/call GetWidthMethod
  //- @width ref GetWidthMethod
  //- @"[box width]" childof Main
  //- @"[box width]".node/kind anchor
  int b = [box width];

  return 0;
}
