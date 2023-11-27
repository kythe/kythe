// Tests that we index `using` decls that alias template names.
// This demonstrates incorrect behavior; see issue #5935
namespace ns {
//- @S defines/binding TemplateS
template <typename T> struct S {};
}
// NB: using declaration cannot refer to a template specialization
//- @S ref TemplateS
using ns::S;
//- @"S<int>" ref TemplateSInt
//- TemplateSInt param.0 TemplateS
//- @S ref TemplateS
S<int> s;
