// We index function template declarations with parameter pack arguments.
//- @Ts defines/binding PackTvar
template <typename... Ts>
//- @Ts ref PackTvar
//- @ts defines/binding PackParam
//- PackParam typed PackTvar
void f(Ts... ts) { }
