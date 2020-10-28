// Verifies that structs are properly handled by the indexer

//- @_TestStruct defines/binding Struct
//- Struct.node/kind record
//- Struct.complete definition
//- Struct.subkind struct
struct _TestStruct {
    //- @test_field defines/binding StructField
    //- StructField childof Struct
    //- StructField.node/kind variable
    //- StructField.complete definition
    //- StructField.subkind field
    test_field: String,
}

impl _TestStruct {
    //- @_test defines/binding TestFn
    //- TestFn.node/kind function
    //- TestFn.complete definition
    //- TestFn childof Struct
    fn _test() {}
}

fn main(){}
