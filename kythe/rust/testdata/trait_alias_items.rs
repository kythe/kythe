#![feature(trait_alias)]
#![feature(custom_inner_attributes)]
#![rustfmt::skip]

trait Bar {}

//- @Foo defines/binding Foo
//- Foo.node/kind talias
//- FooRange defines Foo
//- FooRange.loc/start @^"trait"
//- FooRange.loc/end @$";"
//- Foo code FooCode
//- FooCode.pre_text "trait Foo = \nwhere\n    Self: Bar + Copy,"
trait Foo = Bar + Copy;

//- @Generic defines/binding Generic
//- @T defines/binding T
//- T.node/kind tvar
//- Generic tparam.0 Self
//- Generic tparam.1 T
trait Generic<T> =
    //- @T ref T
    Iterator<Item = T>;
