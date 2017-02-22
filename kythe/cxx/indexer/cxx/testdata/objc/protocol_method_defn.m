// Checks protocol methods are children of the protocol.

//- @Shape defines/binding ShapeProto
@protocol Shape

//- @name defines/binding Name
//- Name childof ShapeProto
-(int)name;

//- @fitsIn defines/binding FitsIn
//- FitsIn childof ShapeProto
-(int)fitsIn:(id<Shape>)shape;

@end
