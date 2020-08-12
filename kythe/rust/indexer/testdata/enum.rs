// Verifies that enums are emitted properly

//- @_TupleEnum defines/binding TupleEnum
//- TupleEnum.node/kind sum
//- TupleEnum.complete definition
//- TupleEnum.subkind enum
enum _TupleEnum {
    //- @TupleVariant defines/binding TupleVariant
    //- TupleVariant childof TupleEnum
    //- TupleVariant.node/kind record
    //- TupleVariant.complete definition
    //- TupleVariant.subkind tuplevariant
    //- @String defines/binding TupleFieldString
    //- TupleFieldString childof TupleVariant
    //- TupleFieldString.node/kind variable
    //- TupleFieldString.complete definition
    //- TupleFieldString.subkind field
    TupleVariant(String),
}

//- @_ConstantEnum defines/binding ConstantEnum
enum _ConstantEnum {
    //- @Constant1 defines/binding EnumConstant1
    //- EnumConstant1 childof ConstantEnum
    //- EnumConstant1.node/kind constant
    Constant1,
    //- @Constant2 defines/binding EnumConstant2
    //- EnumConstant2 childof ConstantEnum
    //- EnumConstant2.node/kind constant
    Constant2 = 5,
}

//- @_StructEnum defines/binding StructEnum
enum _StructEnum {
    //- @Struct defines/binding StructVariant
    //- StructVariant childof StructEnum
    //- StructVariant.node/kind record
    //- StructVariant.complete definition
    //- StructVariant.subkind structvariant
    //- @test_field defines/binding StructField
    //- StructField childof StructVariant
    //- StructField.node/kind variable
    //- StructField.complete definition
    //- StructField.subkind field
    Struct { test_field: String },
}
fn main() {}
