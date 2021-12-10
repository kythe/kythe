// We index constexpr variable templates.
//- VariableTemplate defines/binding VT
//- VT.node/kind variable
template <typename T> constexpr T VariableTemplate{};
//- @VariableTemplate ref VT
//- @auto ref IntBuiltin
auto TemplateValue = VariableTemplate<int>;
//- TApp.node/kind tapp
//- TApp param.0 VT
//- TApp param.1 IntBuiltin
//- VTVar.node/kind variable
//- VTVar instantiates TApp
//- VTVar specializes TApp
