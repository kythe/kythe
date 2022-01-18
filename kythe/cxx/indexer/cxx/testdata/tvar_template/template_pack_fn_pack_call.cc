// We index calls to function templates with parameter pack arguments.

//- @Ts defines/binding FTs
template <typename... Ts>
//- @S defines/binding AbsS
void S(Ts... ts);

//- @Ts defines/binding GTs
template <typename... Ts>
//- @g defines/binding FnTG
void g(Ts... ts) {
  //- SCall childof FnTG
  //- SCall ref/call LookupS
  auto s = S<Ts...>(ts...);
}

//- LookupS.text "S<Ts...>"
