// Test that the type of a property links to the class node.

//- @Fwd defines/binding FwdDecl
@class Fwd;

//- @Data defines/binding DataDecl
@interface Data
@end

//- @Data defines/binding DataDefn
@implementation Data
@end

//- @Obj defines/binding ObjDecl
@interface Obj
@end

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

//- @cdata defines/binding CDataDecl
//- CDataDecl childof BoxDecl
//- @Data ref DataDefn
@property Data *cdata;

//- @cobj defines/binding CObjDecl
//- CObjDecl childof BoxDecl
//- @Obj ref ObjDecl
@property(nonatomic) Obj *cobj;

//- @cfwd defines/binding CFwdDecl
//- CFwdDecl childof BoxDecl
//- @Fwd ref vname("Fwd#c#t",_,_,_,_)
@property Fwd *cfwd;

@end

//- @Box defines/binding BoxImpl
//- BoxImpl.node/kind record
//- BoxImpl.subkind class
//- BoxImpl.complete definition
@implementation Box

@synthesize width = _testwidth;

@end

