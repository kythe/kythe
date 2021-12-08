// Checks that templates can accept multiple typename arguments.

template
//- @T defines/binding AbsT
//- @S defines/binding AbsS
<typename T, typename S>
//- @C defines/binding CDecl1
class C;

template
//- @N defines/binding AbsN
//- @V defines/binding AbsV
<typename N, typename V>
//- @C defines/binding CDecl2
class C;

template
//- @W defines/binding AbsW
//- @X defines/binding AbsX
<typename W, typename X>
//- @C defines/binding CDefn
//- @C completes/uniquely CDecl2
//- @C completes/uniquely CDecl1
class C { };

//- CDecl1 param.0 AbsT
//- CDecl1 param.1 AbsS
//- CDecl2 param.0 AbsN
//- CDecl2 param.1 AbsV
//- CDefn param.0 AbsW
//- CDefn param.1 AbsX
