// We index namespaces.
//- @ns ref NamespaceNS
//- NamespaceNS.node/kind package
//- NamespaceNS.subkind namespace
namespace ns {
  //- @cns ref NamespaceCNS
  namespace cns {
  }
}
//- NamespaceCNS childof NamespaceNS

//- @cns ref ExternalCNS
//- !{ ExternalCNS childof NamespaceNS }
namespace cns {
}

//- @ns ref NamespaceNS
namespace ns {
}
