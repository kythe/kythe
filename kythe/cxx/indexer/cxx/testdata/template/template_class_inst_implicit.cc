// Checks the behavior of implicit class template instantiations.
using ExternalDef = int;
//- @C defines/binding TemplateC
//- @T defines/binding TvarT
template <typename T> struct C {
//- @X defines/binding ExternalDefAlias
//- @X.loc/start XStart
//- @X.loc/end XEnd
  using X = ExternalDef;
//- @Y defines/binding TvarAlias
//- @Y.loc/start YStart
//- @Y.loc/end YEnd
  using Y = T;
};
C<int> x;
//- ImpX specializes TAppCInt
//- TAppCInt.node/kind tapp
//- TAppCInt param.0 TemplateC
//- WraithXAnchor childof/context ImpX
//- WraithXAnchor.node/kind anchor
//- WraithXAnchor.loc/start XStart
//- WraithXAnchor.loc/end XEnd
//- WraithXAnchor defines/binding ExternalDefAlias
//- TvarAlias aliases TvarT
//- WraithYAnchor childof/context ImpX
//- WraithYAnchor.loc/start YStart
//- WraithYAnchor.loc/end YEnd
//- WraithYAnchor defines/binding WraithAlias
//- WraithAlias aliases vname("int#builtin",_,_,_,"c++")
