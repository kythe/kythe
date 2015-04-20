// Checks that we do the right thing with system builtin functions.
#define TRU(x) __builtin_expect(x,1)
#define LIE(x) __builtin_expect(x,0)
int x = TRU(1);
//- @"__builtin_expect" ref vname("__builtin_expect#n#builtin",_,_,_,_)
int y = __builtin_expect(1,1);
