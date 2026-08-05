//- @FOO defines/binding Foo
//- Foo.node/kind constant
//- FooRange defines Foo
//- FooRange.loc/start @^"const"
//- FooRange.loc/end @$";"
//- Foo code FooCode
//- FooCode.pre_text "const FOO: u32"
const FOO: u32 = 10 + 2;
