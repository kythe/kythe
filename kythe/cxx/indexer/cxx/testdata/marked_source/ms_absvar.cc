// We emit MarkedSource for tvars.
//- @T defines/binding TvarT
//- AbsvarT.node/kind tvar
//- AbsvarT code TRoot
//- TRoot.kind "IDENTIFIER"
//- TRoot.pre_text "T"
template <typename T>
T t;

namespace ns {
//- @T defines/binding TvarTS
//- AbsvarTS.node/kind tvar
//- AbsvarTS code TSRoot
//- TSRoot.kind "IDENTIFIER"
//- TSRoot.pre_text "T"
template <typename T>
T s;
}
