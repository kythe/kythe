//- @Enum defines/binding Enum
//- Enum.node/kind sum
//- Enum.subkind enum
//- EnumRange defines Enum
//- EnumRange.loc/start @^"enum"
//- Enum code EnumCode
//- EnumCode.pre_text "enum Enum"
enum Enum {
    //- @Foo defines/binding Foo
    //- Foo.node/kind record
    //- Foo extends Enum
    //- Foo code FooCode
    //- FooCode.pre_text "Foo"
    Foo,

    //- @TupleLike defines/binding TupleLike
    //- TupleLike.node/kind record
    //- TupleLike extends Enum
    //- TUpleLike code TupleLikeCode
    //- TupleLikeCode.pre_text "TupleLike(u8, i16)"
    TupleLike(u8, i16),

    //- @StructLike defines/binding StructLike
    //- StructLike.node/kind record
    //- StructLike extends Enum
    //- StructLike code StructLikeCode
    //- StructLikeCode.pre_text "StructLike"
    StructLike {
        //- @x defines/binding X
        //- X.node/kind variable
        //- X.subkind field
        //- X childof StructLike
        x: i32,
    },
    //- EnumRange.loc/end @$"}"
}

//- @Generic defines/binding Generic
//- @T defines/binding T
//- T.node/kind tvar
//- Generic tparam.0 T
//- Generic code GenericCode
//- GenericCode.pre_text "enum Generic<T>"
enum Generic<T> {
    //- @T ref T
    Bar(T),
    Baz,
}
