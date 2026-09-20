#[rustfmt::skip] fn func() { let var: i32; }
//       1         2         3
//34567890123456789012345678901234567890
//- X = vname("[nested_items.rs]/=func/=var[33:36]",_,_,_,_).node/kind variable

// rust-analyzer's HIR does not track most nested item definitions (e.g., functions defined inside
// of other functions). We 'discover' these definitions names/refs pointing to them.

// @parent defines/binding Parent = vname("[nested_items.rs]/=parent",_,_,_,_)
fn parent() {
    //- @child defines/binding Child
    //- Child.node/kind function
    fn child() {}

    // Function `child` defined above is only found and indexed thanks to the name ref (function
    // invocation) below that points to it.
    //- @child ref Child
    child();

    //- @unused_child defines/binding UnusedChild
    fn unused_child() {}

    //- @variable defines/binding Variable
    //- Variable.node/kind variable
    let variable: () = ();
    //- @unused_var defines/binding UnusedVar
    //- @variable ref Variable
    let unused_var = variable;

    //- @Struct defines/binding Struct
    //- Struct.node/kind record
    struct Struct;
}
