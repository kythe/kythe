// Verifies that implicit modules have an anchor at the top of the file and
// exlicit modules have an anchor over their name

mod test;

//- ImplicitModAnchor.node/kind anchor
//- ImplicitModAnchor.loc/start 0
//- ImplicitModAnchor.loc/end 0
//- ImplicitModAnchor defines/implicit ImplicitMod
//- ImplicitMod.node/kind record
//- ImplicitMod.subkind module
//- ImplicitMod.complete definition

//- @main defines/binding MainFn
//- MainFn childof ImplicitMod
fn main() {
    explicit_module::hello_world();
}

//- @explicit_module defines/binding ExplicitMod
//- ExplicitMod.node/kind record
//- ExplicitMod.subkind module
//- ExplicitMod.complete definition
mod explicit_module {
    //- @hello_world defines/binding HelloFn
    //- HelloFn childof ExplicitMod
    pub fn hello_world() {
        println!("Hello, world!");
    }
}
