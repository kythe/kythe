// We index C++17 nested namespaces.

//- @A ref NamespaceA
//- @B ref NamespaceB
//- @C ref NamespaceC
//- NamespaceA.node/kind package
//- NamespaceA.subkind namespace
//- NamespaceB.node/kind package
//- NamespaceB.subkind namespace
//- NamespaceC.node/kind package
//- NamespaceC.subkind namespace
//- NamespaceB childof NamespaceA
//- NamespaceC childof NamespaceB
namespace A::B::C {
}
