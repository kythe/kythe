// Checks that the protocols are sorted correctly in the union type even if
// their node id has been hashed because it is very long.

// Let's get a handle on the id type
//- IdTy aliases IdPtrTy
//- IdPtrTy.node/kind tapp
//- IdPtrTy param.0 vname("ptr#builtin", _, _, _, _)
//- IdPtrTy param.1 vname("id#builtin", _, _, _, _)

//- @ColorVeryLongNameThatShouldBeHashed11111111111111111111111111111111111111111111111111 defines/binding ColorProto
@protocol ColorVeryLongNameThatShouldBeHashed11111111111111111111111111111111111111111111111111
@end

//- @ShapeVeryLongNameThatShouldBeHashed11111111111111111111111111111111111111111111111111 defines/binding ShapeProto
@protocol ShapeVeryLongNameThatShouldBeHashed11111111111111111111111111111111111111111111111111
//- @"matches" defines/binding MatchesDecl
//- MatchesDecl childof ShapeProto
//- @param1 defines/binding MatchesParam1
//- MatchesParam1 typed ShapeParamType
//- @param2 defines/binding MatchesParam2
//- MatchesParam2 typed MatchesParam2Type
//- MatchesParam2Type.node/kind tapp
//- MatchesParam2Type param.0 vname("ptr#builtin", _, _, _, _)
//- MatchesParam2Type param.1 AnonShapeColorProtoType
//- AnonShapeColorProtoType.node/kind tapp
//- AnonShapeColorProtoType param.0 vname("TypeUnion#builtin", _, _, _, _)
//- AnonShapeColorProtoType param.1 ColorProto
//- AnonShapeColorProtoType param.2 ShapeProto
-(int)matches:(id<ShapeVeryLongNameThatShouldBeHashed11111111111111111111111111111111111111111111111111>)param1 withColor:(id<ShapeVeryLongNameThatShouldBeHashed11111111111111111111111111111111111111111111111111, ColorVeryLongNameThatShouldBeHashed11111111111111111111111111111111111111111111111111>)param2;

// Test that the order of protocol arguments does not matter.
//- @"bar" defines/binding BarDecl
//- BarDecl childof ShapeProto
//- @param1 defines/binding BarParam1
//- BarParam1 typed MatchesParam2Type
-(int)bar:(id<ColorVeryLongNameThatShouldBeHashed11111111111111111111111111111111111111111111111111, ShapeVeryLongNameThatShouldBeHashed11111111111111111111111111111111111111111111111111>)param1;

@end

int main(int argc, char **argv) {
  return 0;
}
