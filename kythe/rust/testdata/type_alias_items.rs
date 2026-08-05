#![feature(custom_inner_attributes)]
#![rustfmt::skip]

//- @Foo defines/binding Foo
//- Foo.node/kind talias
//- FooRange defines Foo
//- FooRange.loc/start @^"pub"
//- FooRange.loc/end @$";"
//- Foo code FooCode
//- FooCode.pre_text "pub type Foo = String"
pub type Foo = String;

//- @Generic defines/binding Generic
//- Generic.node/kind talias
//- @T defines/binding T
//- T.node/kind tvar
//- Generic tparam.0 T
//- Generic code GenericCode
//- GenericCode.pre_text "type Generic<T> = (T, u8)"
type Generic<T>
    //- @T ref T
    = (T, u8);
