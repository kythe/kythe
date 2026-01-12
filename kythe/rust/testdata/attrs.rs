#![feature(custom_inner_attributes)]
#![rustfmt::skip]

//- @module defines/binding Module
//- Module.tag/deprecated _
#[deprecated] mod module {}

enum Enum {
    //- @Variant defines/binding Variant
    //- Variant.tag/deprecated "eggs"
    #[deprecated = "eggs"] Variant,
}

//- @my_macro defines/binding MyMacro
//- MyMacro.tag/deprecated "since 1.0"
#[deprecated(since = "1.0")] macro_rules! my_macro { () => {} }

//- @function defines/binding Function
//- Function.tag/deprecated "since now: sausages"
#[deprecated(since = "now", note = "sausages")] fn function() {}

struct Struct {
    //- @field defines/binding Field
    //- Field.tag/deprecated "bacon"
    #[deprecated(note = "bacon")] field: u8,
}
