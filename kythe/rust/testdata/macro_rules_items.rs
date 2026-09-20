#![feature(custom_inner_attributes)]
#![rustfmt::skip]

//- @foo defines/binding Foo
//- Foo.node/kind macro
//- FooRange defines Foo
//- FooRange.loc/start @^macro
macro_rules! foo {
    ($x:ident) => {
        const $x: isize = 42;
    }
    //- FooRange.loc/end @$"}"
}

//- @foo ref Foo
foo!(bar);

// Macros can be redefined, each macro gets a distinct vname.
// The macro that escapes via #[macro_use] is the last.
#[macro_use]
mod textual {
    //- @txtm defines/binding Txt1
    macro_rules! txtm { () => {} }
    //- @txtm ref Txt1
    txtm!();
    //- @txtm defines/binding Txt2
    macro_rules! txtm { () => {} }
    //- @txtm ref Txt2
    txtm!();
}
//- @txtm ref Txt2
//- ! { @txtm ref Txt1 }
txtm!();

// The exported macro gets a "canonical" vname, others get positional ones.
mod exported {
    //- @expm defines/binding Exp1
    macro_rules! expm { () => {} }
    //- @expm ref Exp1
    expm!();
    //- @expm defines/binding Hi2 = vname("[macro_rules_items.rs]/~exported/!expm",_,_,_,_)
    #[macro_export] macro_rules! expm { () => {} }
    //- @expm ref Exp2
    expm!();
    //- @expm defines/binding Exp3
    macro_rules! expm { () => {} }
    //- @expm ref Exp3
    expm!();
}
//- @expm ref Exp2
//- ! { @expm ref Exp1 }
//- ! { @expm ref Exp3 }
expm!();
