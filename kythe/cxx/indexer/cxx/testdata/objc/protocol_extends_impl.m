// Checks that a class that implements a protocol that is inherited from
// another protocol overrides the right nodes.

//- @Shape defines/binding ShapeProto
//- ShapeProto.node/kind interface
@protocol Shape
//- @volume defines/binding VolumeDecl
-(int) volume;
@end

//- @Box defines/binding BoxProto
//- BoxProto.node/kind interface
//- BoxProto extends ShapeProto
//- @Shape ref ShapeProto
@protocol Box<Shape>
//- @print defines/binding PrintDecl
-(void) print;
@end

@interface Obj
@end

//- @Package defines/binding PackageDecl
//- PackageDecl extends BoxProto
//- @Box ref BoxProto
@interface Package : Obj<Box>
@end

@implementation Package
//- @volume defines/binding VolumeImpl
//- VolumeImpl overrides VolumeDecl
-(void) volume {
}

//- @print defines/binding PrintImpl
//- PrintImpl overrides PrintDecl
-(void) print {
}
@end
