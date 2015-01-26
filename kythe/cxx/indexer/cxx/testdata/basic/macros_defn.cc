// Tests basic macro definitions.
//- @ID defines MacroID
//- MacroID named vname("ID#m",_,_,_,_)
#define ID(x) (x)
//- @SYMBOL defines MacroSymbol
//- MacroSymbol named vname("SYMBOL#m",_,_,_,_)
#define SYMBOL 1
//- @SYMBOL defines OtherMacroSymbol
//- OtherMacroSymbol named vname("SYMBOL#m",_,_,_,_)
#define SYMBOL 2
