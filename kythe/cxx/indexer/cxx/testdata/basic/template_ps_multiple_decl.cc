// Checks that multiple declarations of a partial specialization are recorded.
template
<typename T, typename S>
//- @C defines CDecl
class C;

template
//- 
<typename V>
//- @C defines CVDecl
class C
<int, V>;

template
<typename W>
//- @C defines CWDecl
class C
<int, W>;

//- CWDecl named CName
//- CVDecl named CName
//- CWDecl specializes TAppCNameW
//- CVDecl specializes TAppCNameV
//- TAppCNameW.node/kind tapp
//- TAppCNameV.node/kind tapp
//- TAppCNameW param.0 NominalC
//- TAppCNameV param.0 NominalC
//- NominalC.node/kind tnominal
//- NominalC named CName