// We index calls to function templates with parameter pack arguments.

//- @Ts defines/binding FTs
template <typename... Ts>
//- @S defines/binding AbsS
void S(Ts... ts);

//- @Ts defines/binding GTs
template <typename... Ts>
//- @g defines/binding AbsG
//- FnTG childof AbsG
void g(Ts... ts) {
  //- SCall childof FnTG
  //- SCall ref/call LookupSC
  auto s = S<Ts...>(ts...);
}

//- LookupSC.node/kind callable
//- LookupS callableas LookupSC
//- LookupS.text "S<Ts...>"
