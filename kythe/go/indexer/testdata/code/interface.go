// Package iface tests code facts for an interface type.
package iface

//- @Thinger defines/binding Thinger
//- Thinger code ThingerCode
//-
//- //--------------------------------------------------
//- ThingerCode child.1 THC1
//- THC1.kind "BOX"
//-
//- THC1 child.0 THC1Context
//- THC1Context.kind "CONTEXT"
//- THC1Context.post_child_text "."
//- THC1Context child.0 THC1ContextID
//- THC1ContextID.kind "IDENTIFIER"
//- THC1ContextID.pre_text "iface"
//-
//- THC1 child.1 THC1Ident
//- THC1Ident.kind "IDENTIFIER"
//- THC1Ident.pre_text "Thinger"
//-
//- //--------------------------------------------------
//- ThingerCode child.2 THC2
//- THC2.kind "TYPE"
//- THC2.pre_text " "
//-
//- //--------------------------------------------------
//- ThingerCode child.3 THC3
//- THC3.kind "TYPE"
//- THC3.pre_text "interface {...}"
type Thinger interface {
	//- @Thing defines/binding Method
	//- Method code MethodCode
	//-
	//- //--------------------------------------------------
	//- MethodCode child.0 MC0
	//- MC0.kind "TYPE"
	//- MC0.pre_text "func "
	//-
	//- //--------------------------------------------------
	//- MethodCode child.1 MC1
	//- MC1.kind "PARAMETER"
	//- MC1.pre_text "("
	//- MC1.post_text ") "
	//- MC1 child.0 MCRecv
	//- MCRecv.kind "TYPE"
	//- MCRecv.pre_text "Thinger"
	//-
	//- //--------------------------------------------------
	//- MethodCode child.2 MCIdent
	//- MCIdent child.0 MCContext
	//-
	//- MCContext.kind "CONTEXT"
	//- MCContext child.0 MCContextPkg
	//- MCContextPkg.kind "IDENTIFIER"
	//- MCContextPkg.pre_text "iface"
	//-
	//- MCContext child.1 MCContextType
	//- MCContextType.kind "IDENTIFIER"
	//- MCContextType.pre_text "Thinger"
	//-
	//- MCIdent child.1 MCName
	//- MCName.kind "IDENTIFIER"
	//- MCName.pre_text "Thing"
	//-
	//- //--------------------------------------------------
	//- MethodCode child.3 MCParams
	//- MCParams.kind "PARAMETER_LOOKUP_BY_PARAM"
	//- MCParams.pre_text "("
	//- MCParams.post_child_text ", "
	//- MCParams.post_text ")"
	//- MCParams.lookup_index 1
	Thing()
}
