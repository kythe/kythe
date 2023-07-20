// Checks that multiple declarations of a partial specialization are recorded.
template
<typename T, typename S>
//- @C defines/binding CDecl
class C;

template
//- 
<typename V>
//- @C defines/binding CVDecl
class C
<int, V>;

template
<typename W>
//- @C defines/binding CWDecl
class C
<int, W>;

//- CWDecl specializes TAppCNameW
//- CVDecl specializes TAppCNameV
//- TAppCNameW.node/kind tapp
//- TAppCNameV.node/kind tapp
//- TAppCNameW param.0 CDecl
//- TAppCNameV param.0 CDecl
//- CDecl.node/kind record
