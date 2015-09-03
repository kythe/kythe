// Tests basic macro definitions.
//- @ID defines/binding MacroID
//- MacroID named vname("ID#m",_,_,_,_)
#define ID(x) (x)
//- @SYMBOL defines/binding MacroSymbol
//- MacroSymbol named vname("SYMBOL#m",_,_,_,_)
#define SYMBOL 1
//- @SYMBOL defines/binding OtherMacroSymbol
//- OtherMacroSymbol named vname("SYMBOL#m",_,_,_,_)
#define SYMBOL 2
