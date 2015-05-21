// We index constexpr variable templates.
//- VariableTemplate defines VTAbs
//- VTAbs.node/kind abs
template <typename T> constexpr T VariableTemplate{};
//- @VariableTemplate ref VTVar
//- @auto ref IntBuiltin
auto TemplateValue = VariableTemplate<int>;
//- TApp.node/kind tapp
//- TApp param.0 VTAbs
//- TApp param.1 IntBuiltin
//- VTVar.node/kind variable
//- VTVar instantiates TApp
//- VTVar specializes TApp
