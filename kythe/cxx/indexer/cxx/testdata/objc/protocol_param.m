// Checks that when protocols are used to specify the type of a parameter, the
// parameter's type is correctly recorded.
//
// Also test that the order of protocol arguments does not matter for the type
// union we construct.
//
// We do not record a source range for id when it is used to specify a set of
// protocols.

// Let's get a handle on the id type
//- IdTy aliases IdPtrTy
//- IdPtrTy.node/kind tapp
//- IdPtrTy param.0 vname("ptr#builtin", _, _, _, _)
//- IdPtrTy param.1 vname("id#builtin", _, _, _, _)

//- @Color defines/binding ColorProto
@protocol Color
@end

//- @Shape defines/binding ShapeProto
@protocol Shape

// This is tested here to make sure the addition of protocol logic did not
// break the basic id type logic.
//- @"name" defines/binding NameDecl
//- NameDecl childof ShapeProto
//- @param1 defines/binding NameParam
//- NameParam typed IdTy
//- @id ref IdTy
-(int)name:(id)param1;

//- @"fitsIn" defines/binding FitsInDecl
//- FitsInDecl childof ShapeProto
//- @shape defines/binding ShapeParam
//- ShapeParam typed ShapeParamType
//- ShapeParamType.node/kind tapp
//- ShapeParamType param.0 vname("ptr#builtin", _, _, _, _)
//- ShapeParamType param.1 ShapeProto
//- @Shape ref ShapeProto
-(int)fitsIn:(id<Shape>)shape;

// Make sure we record the location of id<Shape> even though we already saw that
// type. This catches a bug that was present in the code.
//- @"fits2In" defines/binding Fits2InDecl
//- Fits2InDecl childof ShapeProto
//- @shape defines/binding ShapeParam2
//- ShapeParam2 typed ShapeParamType
//- @Shape ref ShapeProto
-(int)fits2In:(id<Shape>)shape;

// Skip the method defines/binding check because we have to split the
// definition over two lines to make matching the protocols easier.
//- @matches defines/binding MatchesDecl
//- MatchesDecl childof ShapeProto
//- @param1 defines/binding MatchesParam1
//- MatchesParam1 typed ShapeParamType
//- MatchesParam2 typed MatchesParam2Type
//- MatchesParam2Type.node/kind tapp
//- MatchesParam2Type param.0 vname("ptr#builtin", _, _, _, _)
//- MatchesParam2Type param.1 AnonShapeColorProtoType
//- AnonShapeColorProtoType.node/kind tapp
//- AnonShapeColorProtoType param.0 vname("TypeUnion#builtin", _, _, _, _)
//- AnonShapeColorProtoType param.1 ColorProto
//- AnonShapeColorProtoType param.2 ShapeProto
//- @Shape ref ShapeProto
-(int)matches:(id<Shape>)param1 withColor:
//- @param2 defines/binding MatchesParam2
//- @Shape ref ShapeProto
//- @Color ref ColorProto
    (id<Shape, Color>)param2;

// Test that the order of protocol arguments does not matter.
//- @"bar" defines/binding BarDecl
//- BarDecl childof ShapeProto
//- @param1 defines/binding BarParam1
//- BarParam1 typed MatchesParam2Type
//- @Color ref ColorProto
//- @Shape ref ShapeProto
-(int)bar:(id<Color, Shape>)param1;

@end

int main(int argc, char **argv) {
  return 0;
}
