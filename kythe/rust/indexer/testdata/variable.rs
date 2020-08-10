// Verifies that variables and closures are properly handled by the indexer

//- @_TEST_CONSTANT defines/binding Constant
//- Constant.node/kind constant
const _TEST_CONSTANT: u32 = 0;

//- @_TEST_STATIC defines/binding Static
//- Static.node/kind constant
static _TEST_STATIC: &str = "Kythe";

fn main() {
    //- @_test_variable defines/binding Local
    //- Local.node/kind variable
    //- Local.subkind local
    let _test_variable = 0;

    // TODO: Identify declaractions and emit an ".complete = imcomplete" fact
    //- @_test_declaration defines/binding Decl
    //- Decl.node/kind variable
    //- Decl.subkind local
    let _test_declaration: u32;

    //- @_test_closure defines/binding Closure
    //- Closure.node/kind function
    //- @#0closure_variable defines/binding ClosureVariable
    //- ClosureVariable.node/kind variable
    let _test_closure = |closure_variable: i32| -> i32 { closure_variable + 1 };
}
