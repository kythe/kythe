// Checks that templates can accept typename arguments.

template
//- @T defines/binding TT
<typename T>
//- @C defines/binding ACDecl1
class C;

template
//- @S defines/binding TS
<typename S>
//- @C defines/binding ACDecl2
class C;

template
//- @V defines/binding TV
<typename V>
//- @C defines/binding ACDefn
//- @C completes/uniquely ACDecl1
//- @C completes/uniquely ACDecl2
class C { };

//- CDefn childof ACDefn
//- CDecl1 childof ACDecl1
//- CDecl2 childof ACDecl2
//- TT.node/kind tvar
//- TS.node/kind tvar
//- TV.node/kind tvar
//- CDefn tparam.0 TV
//- CDecl2 tparam.0 TS
//- CDecl1 tparam.0 TT
