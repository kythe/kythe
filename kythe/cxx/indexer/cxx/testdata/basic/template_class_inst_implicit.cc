// Checks the behavior of implicit class template instantiations.
using ExternalDef = int;
//- @C defines TemplateC
//- @C childof ThisFile
//- @T defines AbsvarT
template <typename T> struct C {
//- @X defines ExternalDefAlias
//- @X.loc/start XStart
//- @X.loc/end XEnd
  using X = ExternalDef;
//- @Y defines AbsvarAlias
//- @Y.loc/start YStart
//- @Y.loc/end YEnd
  using Y = T;
};
C<int> x;
//- ImpX specializes TAppCInt
//- TAppCInt.node/kind tapp
//- TAppCInt param.0 TemplateC
//- WraithXAnchor childof ImpX
//- WraithXAnchor childof ThisFile
//- WraithXAnchor.node/kind anchor
//- WraithXAnchor.loc/start XStart
//- WraithXAnchor.loc/end XEnd
//- WraithXAnchor defines ExternalDefAlias
//- AbsvarAlias aliases AbsvarT
//- WraithYAnchor childof ImpX
//- WraithYAnchor.loc/start YStart
//- WraithYAnchor.loc/end YEnd
//- WraithYAnchor defines WraithAlias
//- WraithAlias aliases vname("int#builtin",_,_,_,"c++")