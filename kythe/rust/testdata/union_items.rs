#![feature(custom_inner_attributes)]
#![rustfmt::skip]

//- @Union defines/binding Union
//- Union.node/kind sum
//- Union.subkind union
//- UnionRange defines Union
//- UnionRange.loc/start @^"union"
//- Union code UnionCode
//- UnionCode.pre_text "union Union"
union Union {
    //- @field defines/binding Field
    //- Field.node/kind variable
    //- Field.subkind field
    //- Field childof Union
    field: u8,

    //- UnionRange.loc/end @$"}"
}

//- @Generic defines/binding Generic
//- @T defines/binding T
//- @U defines/binding U
//- T.node/kind tvar
//- U.node/kind tvar
//- Generic tparam.0 T
//- Generic tparam.1 U
//- Generic code GenericCode
//- GenericCode.pre_text "union Generic<T, U>\nwhere\n    T: Copy,\n    U: Copy,"
union Generic<T, U>
where T: Copy, U: Copy {
    //- @T ref T
    my_t: T,
    //- @U ref U
    my_u: U,
}
