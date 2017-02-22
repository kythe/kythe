// Checks that method implementations override the declarations from the
// protocol and that implementations of protocols "extend" the protocol.

//- @Shape defines/binding ShapeProto
@protocol Shape

//- @name defines/binding NameDecl
//- NameDecl childof ShapeProto
-(int)name;

//- @fitsIn defines/binding FitsInDecl
//- FitsInDecl childof ShapeProto
-(int)fitsIn:(id<Shape>)shape;

@end

//- @Bounce defines/binding BounceProto
@protocol Bounce
@end

//- @Obj defines/binding ObjDecl
@interface Obj
@end

//- @Obj defines/binding ObjImpl
@implementation Obj
@end

// For the reason why @Obj ref ObjDecl and ObjImpl see
// IndexerASTVisitor::ConnectToSuperClassAndProtocols where we generate the
// node for the superclass.
//
//- @Box defines/binding BoxDecl
//- BoxDecl extends ObjImpl
//- BoxDecl extends ShapeProto
//- BoxDecl extends BounceProto
//- @Obj ref ObjDecl
//- @Obj ref ObjImpl
//- @Shape ref ShapeProto
//- @Bounce ref BounceProto
@interface Box : Obj<Shape,  Bounce>
@end

@implementation Box
//- @name defines/binding NameImpl
//- NameImpl overrides NameDecl
-(int)name {
  return 22;
}

//- @fitsIn defines/binding FitsInImpl
//- FitsInImpl overrides FitsInDecl
-(int)fitsIn:(id<Shape>)shape {
  return 1;
}
@end

int main(int argc, char **argv) {
  return 0;
}
