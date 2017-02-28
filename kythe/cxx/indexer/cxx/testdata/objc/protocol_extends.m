// Checks protocols extend their super-protocol.

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
