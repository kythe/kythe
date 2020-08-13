//- LogModAnchor.node/kind anchor
//- LogModAnchor.loc/start 0
//- LogModAnchor.loc/end 0
//- LogModAnchor defines/implicit LogMod
//- LogMod.node/kind record
//- LogMod.subkind module
//- LogMod.complete definition

//- @hello_world defines/binding HelloWorldFn
//- HelloWorldFn.node/kind function
//- HelloWorldFn.complete definition
pub fn hello_world() {
    println!("Hello, verifier!");
}
