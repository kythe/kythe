// Top-level function calls are blamed on files.
template<class T, T v>
struct constant {
  static constexpr T value = v;
};

//- @foo defines/binding FooFn
constexpr int foo() { return 1; }

//- @foo ref FooFn
//- Anchor=@"foo()" ref/call FooFn
//- Anchor childof FooInit
constant<int, foo()> n;

//- FooInit code FooCode
//- FooCode.kind "IDENTIFIER"
//- FooCode.pre_text
//-     Path="kythe/cxx/indexer/cxx/testdata/template/template_file_call.cc"
//- FooInit.node/kind function
//- FooInit.subkind initializer
//- vname(_,_,_,Path,_) defines/binding FooInit
