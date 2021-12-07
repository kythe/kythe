// We index function template declarations with parameter pack arguments.
//- @Ts defines/binding PackAbsvar
template <typename... Ts>
//- @Ts ref PackAbsvar
//- @ts defines/binding PackParam
//- PackParam typed PackAbsvar
void f(Ts... ts) { }
