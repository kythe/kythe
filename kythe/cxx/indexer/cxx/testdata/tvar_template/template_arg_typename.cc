// Checks that templates can accept typename arguments.

template
//- @T defines/binding TT
<typename T>
//- @C defines/binding CDecl1
class C;

template
//- @S defines/binding TS
<typename S>
//- @C defines/binding CDecl2
class C;

template
//- @V defines/binding TV
<typename V>
//- @C defines/binding CDefn
//- CDecl1 completedby CDefn
//- CDecl2 completedby CDefn
class C { };

//- TT.node/kind tvar
//- TS.node/kind tvar
//- TV.node/kind tvar
//- CDefn tparam.0 TV
//- CDecl2 tparam.0 TS
//- CDecl1 tparam.0 TT
