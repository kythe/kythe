// Verifies that unions are properly handles by the indexer

#[repr(C)]
//- @_TestUnion defines/binding Union
//- Union.node/kind record
//- Union.complete definition
//- Union.subkind union
union _TestUnion {
    //- @f1 defines/binding UnionField
    //- UnionField childof Union
    //- UnionField.node/kind variable
    //- UnionField.complete definition
    //- UnionField.subkind field
    f1: u32,
    f2: f32,
}

fn main() {}
