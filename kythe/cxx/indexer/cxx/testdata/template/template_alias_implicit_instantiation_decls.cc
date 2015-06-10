// Checks that specialization decls are given the right names.
//- @S defines TemplateS
template <typename T> struct S;
//- @T defines NominalAlias
using T = S<float>;
//- TemplateS named TemplateSName
//- NominalAlias aliases TApp
//- TApp param.0 TNominal
//- TNominal.node/kind tnominal
//- TNominal named TemplateSName
