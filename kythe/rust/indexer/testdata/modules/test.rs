//- TestModAnchor.node/kind anchor
//- TestModAnchor.loc/start 0
//- TestModAnchor.loc/end 0
//- TestModAnchor defines/implicit TestMod
//- TestMod.node/kind record
//- TestMod.subkind module
//- TestMod.complete definition
//- TestMod childof ImplicitMod

//- @nothing defines/binding NothingFn
//- NothingFn childof TestMod
fn nothing() {
    ()
}
