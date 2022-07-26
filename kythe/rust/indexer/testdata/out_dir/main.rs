include!(concat!(env!("OUT_DIR"), "/generated.rs"));

fn main() {
    //- @print_hello_world ref FnPrint
    print_hello_world();
}
