// We index calls to ctors of class templates with parameter pack arguments.

//- @Ts defines/binding FTs
template <typename... Ts>
//- @S defines/binding AbsS
struct S {
  S(Ts... ts);
};

//- @Ts defines/binding GTs
template <typename... Ts>
//- @g defines/binding AbsG
//- FnTG childof AbsG
void g(Ts... ts) {
  //- CtorCall ref/call CtorLookup
  //- CtorCall childof FnTG
  auto s = S<Ts...>(ts...);
}

//- CtorLookup.node/kind lookup
//- CtorLookup.text "#ctor"
//- CtorLookup param.0 TAppAbsSTs
//- TAppAbsSTs param.0 AbsS
//- TAppAbsSTs param.1 GTs
