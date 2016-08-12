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

@interface Obj
@end

@implementation Obj
@end

//- @Shape ref ShapeProto
//- @Box extends ShapeProto
@interface Box : Obj<Shape>
@end

@implementation Box
//#- @"name" defines/binding NameImpl
//#- NameImpl overrides Name
-(int)name {
  return 22;
}

//#- @"fitsIn:(id<Shape>)shape " defines/binding FitsInImpl
//#- FitsInImpl overrides FitsIn
-(int)fitsIn:(id<Shape>)shape {
  return 1;
}
@end
