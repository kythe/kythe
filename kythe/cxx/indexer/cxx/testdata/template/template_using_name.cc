// Tests that we index `using` decls that alias template names.
// This demonstrates incorrect behavior; see issue #5935
namespace ns {
//- @S defines/binding TemplateS
template <typename T> struct S {};
}
//- !{@S ref TemplateS}  // TODO(zrlk): Targets missing node.
using ns::S;
// This trips an unimplemented check (TN.UsingTemplate) in
// BuildNodeIdForTemplateName
//- @"S<int>" ref TemplateSInt
//- TemplateSInt param.0 TemplateS
//- !{@S ref TemplateS}  // TODO(zrlk): No anchor for S
S<int> s;
