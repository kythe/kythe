// We emit MarkedSource for absvars.
//- @T defines/binding AbsvarT
//- AbsvarT.node/kind absvar
//- AbsvarT code TRoot
//- TRoot.kind "IDENTIFIER"
//- TRoot.pre_text "T"
template <typename T>
T t;

namespace ns {
//- @T defines/binding AbsvarTS
//- AbsvarTS.node/kind absvar
//- AbsvarTS code TSRoot
//- TSRoot.kind "IDENTIFIER"
//- TSRoot.pre_text "T"
template <typename T>
T s;
}
