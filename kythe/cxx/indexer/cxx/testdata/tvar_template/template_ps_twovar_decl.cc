// Checks that multi-variable partial specialization decls are recorded.
template
<typename T, typename S>
//- @C defines/binding CDecl
class C;

template
//- @U defines/binding AbsU
<typename U>
//- @C defines/binding PCDecl
class C
<int, U>;

//- PCDecl specializes TAppCDeclIntAbsU
//- TAppCDeclIntAbsU.node/kind tapp
//- TAppCDeclIntAbsU param.2 AbsU
//- PCDecl param.0 AbsU
//- TAppCDeclIntAbsU param.0 CNominal
