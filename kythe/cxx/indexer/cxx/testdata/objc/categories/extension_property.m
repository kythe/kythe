// Test that extension properties and ivars are declared and defined correctly.
//
// Note: Extensions are allowed to declare new ivars and properties, categories
// are not.

#import "extension_property.h"

//- @Box defines/binding BoxExt
//- BoxExt extends/category BoxDecl
@interface Box () {
  //- @data defines/binding Data
  //- Data childof BoxExt
  //- Data.node/kind variable
  //- Data.subkind field
  int data;
}

//- ValDecl childof BoxExt
//- @val defines/binding ValDecl
@property int val;

-(int) magicNum;

@end

//- @Box defines/binding BoxImpl
@implementation Box

@synthesize val = data;

-(int) foo {
  //- @data ref/writes Data
  self->data = 300;

  //- @x defines/binding XVar
  //- @val ref ValDecl
  //- @val ref/call _
  int x = self.val;
  return x;
}

-(int) magicNum {
  return 7;
}

@end
