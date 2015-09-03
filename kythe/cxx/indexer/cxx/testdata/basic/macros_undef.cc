// Tests undefining macros.
//- @M defines/binding MacroM
#define M
//- @M undefines MacroM
#undef M
#undef NEVER_DEFINED
//- @M defines/binding OtherMacroM
#define M
//- @M undefines OtherMacroM
#undef M
