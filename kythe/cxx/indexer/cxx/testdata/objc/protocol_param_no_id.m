// Checks that when protocols are used to specify the type of a parameter that
// is a specific type and not id.

//- @Foo defines/binding FooDecl
@interface Foo
@end

//- @Color defines/binding ColorProto
@protocol Color
@end

//- @Shape defines/binding ShapeDecl
@interface Shape



//- @"fitsIn" defines/binding FitsInDecl
//- FitsInDecl childof ShapeDecl
//- @shape defines/binding ShapeParam
//- ShapeParam typed ShapeParamType
//- ShapeParamType param.0 vname("ptr#builtin", _, _, _, _)
//- ShapeParamType param.1 AnonTypeUnion
//- AnonTypeUnion.node/kind tapp
//- AnonTypeUnion param.0 vname("TypeUnion#builtin", _, _, _, _)
//- AnonTypeUnion param.1 FooDecl
//- AnonTypeUnion param.2 ColorProto
//- ShapeParamType.node/kind tapp
//- @Foo ref FooDecl
//- @Color ref ColorProto
-(int)fitsIn:(Foo<Color>*)shape;

@end

int main(int argc, char **argv) {
  return 0;
}
