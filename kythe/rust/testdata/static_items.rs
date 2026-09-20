//- @FOO defines/binding Foo
//- Foo.node/kind variable
//- FooRange defines Foo
//- FooRange.loc/start @^"static"
//- FooRange.loc/end @$";"
//- Foo code FooCode
//- FooCode.pre_text "static FOO: u32"
static FOO: u32 = 10 + 2;
