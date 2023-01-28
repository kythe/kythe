// Checks that templates can accept multiple typename arguments.

template
//- @T defines/binding TT
//- @S defines/binding TS
<typename T, typename S>
//- @C defines/binding CDecl1
class C;

template
//- @N defines/binding TN
//- @V defines/binding TV
<typename N, typename V>
//- @C defines/binding CDecl2
class C;

template
//- @W defines/binding TW
//- @X defines/binding TX
<typename W, typename X>
//- @C defines/binding CDefn
//- CDecl1 completedby CDefn
//- CDecl2 completedby CDefn
class C { };

//- CDecl1 tparam.0 TT
//- CDecl1 tparam.1 TS
//- CDecl2 tparam.0 TN
//- CDecl2 tparam.1 TV
//- CDefn tparam.0 TW
//- CDefn tparam.1 TX
