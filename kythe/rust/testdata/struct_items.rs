#![feature(custom_inner_attributes)]
#![rustfmt::skip]

//- @Struct defines/binding Struct
//- Struct.node/kind record
//- StructRange defines Struct
//- StructRange.loc/start @^"struct"
//- Struct code StructCode
//- StructCode.pre_text "struct Struct"
struct Struct {
    //- @foo defines/binding Foo
    //- Foo.node/kind variable
    //- Foo.subkind field
    //- Foo childof Struct
    //- Foo code FooCode
    //- FooCode.pre_text "pub foo: [u8; 3]"
    pub foo: [u8; 3],

    //- StructRange.loc/end @$"}"
}

//- @TupleLike defines/binding TupleLike
//- TupleLike.node/kind record
//- TupleLikeRange defines TupleLike
//- TupleLikeRange.loc/start @^"struct"
//- TupleLike code TupleLikeCode
//- TupleLikeCode.pre_text "struct TupleLike(Struct)"
struct TupleLike(
    //- @Struct ref Struct
    Struct,
    //- TupleLikeRange.loc/end @$";"
);

//- @Generic defines/binding Generic
//- @T defines/binding T
//- @U defines/binding U
//- T.node/kind tvar
//- U.node/kind tvar
//- Generic tparam.0 T
//- Generic tparam.1 U
//- Generic code GenericCode
//- GenericCode.pre_text "struct Generic<T, U>"
struct Generic<T, U> {
    //- @T ref T
    my_t: T,
    //- @U ref U
    my_u: U,
}
