// Verifies that cross-references work properly

//- MainModAnchor.node/kind anchor
//- MainModAnchor.loc/start 0
//- MainModAnchor.loc/end 0
//- MainModAnchor defines/implicit MainMod
//- MainMod.node/kind record
//- MainMod.subkind module
//- MainMod.complete definition

////- @log ref LogMod
mod log;

//- @NUM defines/binding NumConst
//- NumConst.node/kind constant
const NUM: u32 = 0;

//- @KYTHE_STR defines/binding Static
//- Static.node/kind constant
static KYTHE_STR: &str = "Kythe";

//- @CustomType defines/binding Type
//- Type.node/kind talias
type CustomType = u32;

//- @arg1 defines/binding Arg1
//- Arg1.node/kind variable
//- Arg1.subkind local
//- @arg2 defines/binding Arg2
//- Arg2.node/kind variable
//- Arg2.subkind local
//- @add defines/binding AddFn
//- AddFn.node/kind function
//- AddFn.complete definition
fn add(arg1: u32, arg2: u32) -> u32 {
    //- @arg1 ref Arg1
    //- @arg2 ref Arg2
    arg1 + arg2
}

//- @TestTrait defines/binding Trait
trait TestTrait {
    //- @hello defines/binding TraitHelloFn
    fn hello(){}
}

//- @TestStruct defines/binding Struct
struct TestStruct {
    //- @test_field defines/binding Field
    //- @CustomType ref Type
    pub test_field: CustomType,
}

//- @TestTrait ref Trait
//- @TestStruct ref Struct
impl TestTrait for TestStruct {
    //- @hello defines/binding HelloFn
    //- HelloFn.node/kind function
    fn hello() {
        println!("Hello, Rust xrefs!");
    }
}

fn main() {
    //- @var1 defines/binding Var1
    let var1 = 1;
    //- @var2 defines/binding Var2
    let var2 = 2;
    //- @var3 defines/binding Var3
    //- @var1 ref Var1
    //- @var2 ref Var2
    //- @add ref AddFn
    let var3 = add(var1, var2);

    //- @var3 ref Var3
    println!("{}", var3);

    //- @var3 ref Var3
    //- @NUM ref NumConst
    let var4 = var3 + NUM;

    //- @KYTHE_STR ref Static
    println!("{}", KYTHE_STR);

    //- @TestStruct ref Struct
    //- @var5 defines/binding Var5
    let var5 = TestStruct {
        //- @test_field ref Field
        test_field: var4,
    };

    //- @var5 ref Var5
    //- @test_field ref Field
    println!("{}", var5.test_field);

    // TODO: Make this reference the implementation on the method,
    // not the trait definition.
    //- @"hello" ref TraitHelloFn
    //- @TestStruct ref Struct
    TestStruct::hello();

    ////- @log ref LogMod
    //- @hello_world ref HelloWorldFn
    log::hello_world();
}
