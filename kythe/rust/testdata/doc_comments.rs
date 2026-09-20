#![feature(custom_inner_attributes)]
#![rustfmt::skip]

//- @+6foo defines/binding Foo
//- FooDoc documents Foo
//- FooDoc.node/kind doc
//- FooDoc.text "Hello!"

/// Hello!
pub fn foo() {}

enum Enum {
    //- @+7Variant defines/binding Variant
    //- VariantDoc documents Variant
    //- VariantDoc.text "# Markdown Title\n**bold** \\[link\\](https://google.com)\nEscape me: \\\\"

    /// # Markdown Title
    /// **bold** [link](https://google.com)
    /// Escape me: \
    Variant,
}
