// Verifies that xrefs work for inline tests (must be compiled with --test)

fn main() {
    println!("Hello, world!");
}

//- @+4"test_fn" defines/binding TestFn
//- TestFn.node/kind function
//- TestFn.complete definition
#[test]
fn test_fn() {
    //- @x defines/binding VarX
    //- VarX.node/kind variable
    //- VarX.subkind local
    let x = 1;
    assert_eq!(x, 1);
}
