// Checks that Objective-C synthesized properties are declared and defined.

//- @Box defines/binding BoxDecl
//- BoxDecl.node/kind record
//- BoxDecl.subkind class
//- BoxDecl.complete complete
@interface Box {
  //- @width defines/binding WidthIVarDecl
  int width;
}

//- @a defines/binding ADecl
@property int a;

//- @b defines/binding BDecl
//- BDecl.node/kind variable
//- BDecl.subkind field
//- @b defines/binding BIvarDecl
//- BIvarDecl.node/kind variable
//- BIvarDecl.subkind field
@property int b;

//- @c defines/binding CDecl
@property int c;

//- @d defines/binding DDecl
@property int d;

-(void) foo;
@end


//- @Box defines/binding BoxImpl
//- BoxImpl.node/kind record
//- BoxImpl.subkind class
//- BoxImpl.complete definition
@implementation Box

@synthesize a = width;

//- @c defines/binding CIVarDecl
@synthesize c;

//- @newvar defines/binding NewvarIVarDecl
@synthesize d = newvar;

-(void) foo {
  //- @width ref/writes WidthIVarDecl
  self->width = 100;

  //- @a ref ADecl
  self.a = 200;

  //- @b ref BDecl
  self.b = 300;
  //- @"_b" ref/writes BIvarDecl
  self->_b = 301;

  //- @c ref CDecl
  self.c = 400;
  //- @c ref/writes CIvarDecl
  self->c = 401;

  //- @d ref DDecl
  self.d = 500;
  //- @newvar ref/writes NewvarIVarDecl
  self->newvar = 501;
}
@end

int main(int argc, char **argv) {
  return 0;
}
