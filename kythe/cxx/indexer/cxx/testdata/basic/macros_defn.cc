// Tests basic macro definitions.
//- @ID defines/binding MacroID
//- MacroID.node/kind macro
#define ID(x) (x)
//- @SYMBOL defines/binding MacroSymbol
//- MacroSymbol.node/kind macro
#define SYMBOL 1
//- @SYMBOL defines/binding OtherMacroSymbol
//- !{@SYMBOL defines/binding MacroSymbol}
//- OtherMacroSymbol.node/kind macro
#define SYMBOL 2
