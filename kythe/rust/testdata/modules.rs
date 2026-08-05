#![feature(custom_inner_attributes)]
#![rustfmt::skip]
//- Top defines/implicit Modules = vname("[modules.rs]","google","","","rust")
//- Top.node/kind anchor
//- Top.loc/start 0
//- Top.loc/end 0
//- Modules.node/kind package

//- @foo defines/binding Foo = vname("[modules.rs]/~foo",_,_,_,_)
//- Foo.node/kind package
//- Foo childof Modules
mod foo {
    //- @bar defines/binding Bar = vname("[modules.rs]/~foo/~bar",_,_,_,_)
    //- Bar.node/kind package
    //- Bar childof Foo
    pub mod bar {
        pub const X: u32 = 42;
    }

    //- @baz defines/binding Baz
    //- BazRange defines Baz
    //- BazRange.loc/start @^"mod"
    mod baz {
        //- BazRange.loc/end @$"}"
    }
}

//- @foo ref Foo
//- @bar ref Bar
const Y: u32 = foo::bar::X;

//- @external ref External = vname("[modules.rs]/~external","google","","","rust")
//- ExternalAnchor = vname("[0:0]","google","","kythe/rust/testdata/external.rs","rust") defines/implicit External
//- External childof Modules
mod external;

//- @external ref External
//- @inner ref Inner
//- Inner childof External
const Z : usize = external::inner::X;
