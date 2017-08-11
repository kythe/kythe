// We shouldn't emit ref edges for implicit conversions or moves.
struct T {
//- @T defines/binding TCtor
  T();
//- @#0T defines/binding TMCtor
  T(T&& from) { }
};
//- FT=@#1T ref TCtor
//- FT.loc/start FTBegin
//- FZero ref TMCtor
//- FZero.loc/start FTBegin
//- FZero.loc/end FTBegin
//- @"T()" ref/call TCtor
//- @"T()" ref/call TMCtor
//- @"T()" childof FnF
//- @f defines/binding FnF
T f() { return T(); }
//- GC=@"f()" ref/call FnF
//- GC ref/call TMCtor
//- GC childof FnG
//- GF=@f ref FnF
//- GF.loc/start GFBegin
//- GZero ref TMCtor
//- GZero.loc/start GFBegin
//- GZero.loc/end GFBegin
//- @g defines/binding FnG
T g() { return f(); }
struct S {
  S(T t) { }
};
S h() {
//- HGC=@"g()" ref/call FnG
//- HGC ref/call TMCtor
//- HGC.loc/start HGCBegin
//- @g ref FnG
//- HZero.loc/start HGCBegin
//- HZero.loc/end HGCBegin
//- HZero ref TMCtor
  return S{g()};
}
