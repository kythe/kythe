// We index namespaces.
//- @ns defines/binding NamespaceNS
//- NamespaceNS.node/kind package
//- NamespaceNS.subkind namespace
namespace ns {
  //- @cns defines/binding NamespaceCNS
  namespace cns {
  }
}
//- NamespaceCNS childof NamespaceNS

//- @cns defines/binding ExternalCNS
//- !{ ExternalCNS childof NamespaceNS }
namespace cns {
}

//- @ns defines/binding NamespaceNS
namespace ns {
}
