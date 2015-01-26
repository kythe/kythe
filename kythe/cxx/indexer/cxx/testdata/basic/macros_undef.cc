// Tests undefining macros.
//- @M defines MacroM
#define M
//- @M undefines MacroM
#undef M
#undef NEVER_DEFINED
//- @M defines OtherMacroM
#define M
//- @M undefines OtherMacroM
#undef M
