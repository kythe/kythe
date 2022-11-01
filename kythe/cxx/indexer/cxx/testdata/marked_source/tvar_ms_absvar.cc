// We emit MarkedSource for tvars.
//- @T defines/binding TvarT
//- TvarT.node/kind tvar
//- TvarT code TRoot
//- TRoot.kind "IDENTIFIER"
//- TRoot.pre_text "T"
template <typename T>
T t;

namespace ns {
//- @T defines/binding TvarTS
//- TvarTS.node/kind tvar
//- TvarTS code TSRoot
//- TSRoot.kind "IDENTIFIER"
//- TSRoot.pre_text "T"
template <typename T>
T s;
}
