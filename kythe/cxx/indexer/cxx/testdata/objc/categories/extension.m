// Test that extensions correctly declare methods and the implementations for
// those methods connect back to the decl in the extension.

#import "extension.h"

//- @Box defines/binding BoxExt
//- BoxExt extends/category BoxDecl
@interface Box ()

//- @magicNum defines/binding MagicNumDecl
//- MagicNumDecl.node/kind function
//- MagicNumDecl.complete incomplete
//- MagicNumDecl childof BoxExt
-(int) magicNum;

@end

//- @Box defines/binding BoxImpl
@implementation Box

-(int) foo {
  return [self magicNum];
  return 20;
}

//- MagicNumDecl completedby MagicNumDefn
//- @magicNum defines/binding MagicNumDefn
//- MagicNumDefn.node/kind function
//- MagicNumDefn.complete definition
//- MagicNumDefn childof BoxImpl
-(int) magicNum {
  return 7;
}

@end
