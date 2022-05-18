#[path = "../relative_module/mod.rs"]
//- @relative_module ref RelativeMod
mod relative_module;

fn main() {
    //- @print_hello_world ref FnPrint
    relative_module::print_hello_world();
}
