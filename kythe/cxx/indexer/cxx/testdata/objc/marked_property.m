// Test marked source for properties.

@class Data;

@interface Box {
  int _testwidth;
}


//- @width defines/binding WidthPropDecl
//- WidthPropDecl code WPCodeRoot
//- WPCodeRoot.pre_text "@property "
//- WPCodeRoot.kind "BOX"
//- WPCodeRoot child.0 WPRootType
//- WPRootType.kind "TYPE"
//- WPRootType.pre_text "int"
//- WPCodeRoot child.1 WPContext
//- WPContext.kind "CONTEXT"
//- WPContext.post_child_text "::"
//- WPContext child.0 WPContextIdent
//- WPContextIdent.kind "IDENTIFIER"
//- WPContextIdent.pre_text "Box"
//- WPCodeRoot child.2 WPIdent
//- WPIdent.kind "IDENTIFIER"
//- WPIdent.pre_text "width"
@property int width;

@end

@implementation Box

@synthesize width = _testwidth;

@end
