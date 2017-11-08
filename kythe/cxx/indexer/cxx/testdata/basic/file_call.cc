// Top-level function calls are blamed on files.

//- @foo defines/binding FooFn
constexpr int foo() { return 1; }

//- @foo ref FooFn
//- Anchor=@"foo()" ref/call FooFn
//- Anchor childof FooInit
static_assert(foo() == 1, "");

//- FooInit code FooCode
//- FooCode.kind "IDENTIFIER"
//- FooCode.pre_text "kythe/cxx/indexer/cxx/testdata/basic/file_call.cc"
//- FooInit.node/kind function
//- FooInit.subkind initializer
//- vname(_,_,_,"kythe/cxx/indexer/cxx/testdata/basic/file_call.cc",_)
//-     defines/binding FooInit
