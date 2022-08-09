//- @print_hello_world=vname(_,_,"bazel-out/bin",_,_) defines/binding FnPrint
//- FnPrint.node/kind function
//- FnPrint.complete definition
pub fn print_hello_world() {
    println!("Hello, world!");
}
