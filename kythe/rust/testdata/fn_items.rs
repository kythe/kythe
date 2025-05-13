#![feature(custom_inner_attributes)]
#![rustfmt::skip]

//- @function defines/binding Function
//- Function.node/kind function
//- FunctionRange defines Function
//- FunctionRange.loc/start @^"pub"
//- Function code FunctionCode
//- FunctionCode.pre_text "pub fn function(foo: u16) -> u16"
pub fn function(
    //- @foo defines/binding Foo
    //- Function param.0 Foo
    //- Foo code FooCode
    //- FooCode.pre_text "foo: u16"
    foo: u16,
) -> u16 {
    foo + 1
    //- FunctionRange.loc/end @$"}"
}

//- @identity defines/binding Identity
//- @T defines/binding T
//- T.node/kind tvar
//- Identity tparam.0 T
fn identity<T>(
    //- @x defines/binding X
    //- Identity param.0 X
    //- @T ref T
    x: T,
//- @T ref T
) -> T {
    //- @x ref X
    x
}
