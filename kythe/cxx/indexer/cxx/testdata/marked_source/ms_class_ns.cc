// Test marked source for c++ classes in namespaces.

namespace ns1 {
//- @Bar defines/binding BarClass
//- BarClass code BarMSRoot
//- BarMSRoot.kind "BOX"
//- BarMSRoot child.0 NSSep
//- BarMSRoot child.1 BarMSID
//- NSSep.kind "CONTEXT"
//- NSSep.post_child_text "::"
//- NSSep child.0 NSName
//- NSName.kind "IDENTIFIER"
//- NSName.pre_text "ns1"
//- BarMSID.kind "IDENTIFIER"
//- BarMSID.pre_text "Bar"
class Bar {
};
};
