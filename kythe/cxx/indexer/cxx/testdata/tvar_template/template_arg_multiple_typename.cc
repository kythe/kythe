// Checks that templates can accept multiple typename arguments.

template
//- @T defines/binding TT
//- @S defines/binding TS
<typename T, typename S>
//- @C defines/binding ACDecl1
class C;

template
//- @N defines/binding TN
//- @V defines/binding TV
<typename N, typename V>
//- @C defines/binding ACDecl2
class C;

template
//- @W defines/binding TW
//- @X defines/binding TX
<typename W, typename X>
//- @C defines/binding ACDefn
//- @C completes/uniquely ACDecl2
//- @C completes/uniquely ACDecl1
class C { };

//- CDecl1 childof ACDecl1
//- CDecl2 childof ACDecl2
//- CDefn childof ACDefn
//- CDecl1 tparam.0 TT
//- CDecl1 tparam.1 TS
//- CDecl2 tparam.0 TN
//- CDecl2 tparam.1 TV
//- CDefn tparam.0 TW
//- CDefn tparam.1 TX
