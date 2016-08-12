// Checks protocols are defined correctly.
//
// todo(salguarnieri) Add commented out verification.

//- @Shape defines/binding ShapeProto
@protocol Shape

//#- Name childof ShapeProto
//- @"name" defines/binding Name
-(int)name;

//#- FitsIn childof ShapeProto
//#- @"fitsIn:(id<Shape>)shape" defines/binding FitsIn
-(int)fitsIn:(id<Shape>)shape;

@end
