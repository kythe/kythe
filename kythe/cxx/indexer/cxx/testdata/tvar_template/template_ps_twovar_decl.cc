// Checks that multi-variable partial specialization decls are recorded.
template
<typename T, typename S>
//- @C defines/binding CDecl
class C;

template
//- @U defines/binding TU
<typename U>
//- @C defines/binding APCDecl
class C
<int, U>;

//- APCDecl specializes TAppCDeclIntAbsU
//- TAppCDeclIntAbsU.node/kind tapp
//- TAppCDeclIntAbsU param.2 TU
//- PCDecl childof APCDecl
//- PCDecl tparam.0 TU
//- TAppCDeclIntAbsU param.0 CNominal
