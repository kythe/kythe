// Verifies that implicit modules have an anchor at the top of the file and
// exlicit modules have an anchor over their name

//- ImplicitMod=vname(_, _, _, "testdata/modules.rs", _).node/kind record
//- ImplicitMod.subkind module
//- ImplicitMod.complete definition

//- ImplicitModAnchor.node/kind anchor
//- ImplicitModAnchor.loc/start 0
//- ImplicitModAnchor.loc/end 0
//- ImplicitModAnchor defines/implicit ImplicitMod

fn main() {
    explicit_module::hello_world();
}

//- ExplicitMod=vname(_, _, _, "testdata/modules.rs", _).node/kind record
//- ExplicitModsubkind module
//- ExplicitMod.complete definition
//- @explicit_module defines/binding ExplicitMod
mod explicit_module {
    pub fn hello_world() {
        println!("Hello, world!");
    }
}
