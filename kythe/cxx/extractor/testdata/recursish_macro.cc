// Checks that the extractor handles cycles in macro definitions (by not
// overflowing) and documents some preprocessor expansion corner cases.
//- @foo defines/binding IntAlias
using foo = int;
//- @bar defines/binding FloatAlias
using bar = float;
//- @foo defines/binding MacroFoo
#define foo bar
//- @bar defines/binding MacroBar
#define bar foo
//- @foo ref/expands MacroFoo
//- @af defines/binding AliasFoo
//- AliasFoo aliases IntAlias
using af = foo;
//- @bar ref/expands MacroBar
//- @ab defines/binding AliasBar
//- AliasBar aliases FloatAlias
using ab = bar;
