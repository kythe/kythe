// Checks that the extractor handles cycles in macro definitions (by not
// overflowing) and documents some preprocessor expansion corner cases.
//- @foo defines IntAlias
using foo = int;
//- @bar defines FloatAlias
using bar = float;
//- @foo defines MacroFoo
#define foo bar
//- @bar defines MacroBar
#define bar foo
//- @foo ref/expands MacroFoo
//- @af defines AliasFoo
//- AliasFoo aliases IntAlias
using af = foo;
//- @bar ref/expands MacroBar
//- @ab defines AliasBar
//- AliasBar aliases FloatAlias
using ab = bar;
