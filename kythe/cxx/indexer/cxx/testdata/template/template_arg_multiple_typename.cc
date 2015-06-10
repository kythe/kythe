// Checks that templates can accept multiple typename arguments.

template
//- @T defines AbsT
//- @S defines AbsS
<typename T, typename S>
//- @C defines CDecl1
class C;

template
//- @N defines AbsN
//- @V defines AbsV
<typename N, typename V>
//- @C defines CDecl2
class C;

template
//- @W defines AbsW
//- @X defines AbsX
<typename W, typename X>
//- @C defines CDefn
//- @C completes/uniquely CDecl2
//- @C completes/uniquely CDecl1
class C { };

//- CDecl1 param.0 AbsT
//- CDecl1 param.1 AbsS
//- CDecl2 param.0 AbsN
//- CDecl2 param.1 AbsV
//- CDefn param.0 AbsW
//- CDefn param.1 AbsX
