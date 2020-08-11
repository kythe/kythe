// Verifies that traits are properly handled by the indexer

//- @TestTrait defines/binding Trait
//- Trait.node/kind interface
trait TestTrait {
    //- @test_method defines/binding TestMethod
    //- TestMethod childof Trait
    fn test_method() {}
}

#[allow(dead_code)]
//- @TestStruct defines/binding Struct
struct TestStruct {}

impl TestTrait for TestStruct {
    //- @test_method defines/binding ImplMethod
    //- ImplMethod childof Struct
    fn test_method() {}
}

fn main() {}
