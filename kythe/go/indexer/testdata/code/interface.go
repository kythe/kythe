// Package iface tests code facts for an interface type.
package iface

//- @Thinger defines/binding Thinger
//- Thinger code TCode
//-
//- TCode.kind "TYPE"
//- TCode child.0 TType
//- TCode child.1 TName
//- TCode child.2 TSpace
//- TCode child.3 TInterface
//-
//- TType.pre_text "type "
//-
//- TName child.0 TContext
//- TName child.1 TIdent
//-
//- TSpace.pre_text " "
//- TInterface.pre_text "interface {...}"
type Thinger interface {
	//- @Thing defines/binding Thing
	//- Thing code MCode
	//-
	//- MCode child.0 MFunc
	//- MCode child.1 MRecv
	//- MCode child.2 MName
	//- MCode child.3 MParams
	//-
	//- MFunc.pre_text "func "
	//-
	//- MRecv.kind "PARAMETER"
	//- MRecv.pre_text "("
	//- MRecv.post_text ") "
	//- MRecv child.0 MRType
	//-
	//- MName.post_child_text "."
	//- MName child.0 MContext
	//- MName child.1 MOwner
	//- MName child.2 MIdent
	//-
	//- MParams.kind "PARAMETER"
	//- MParams.pre_text "()"
	//-
	//- MRType.kind "TYPE"
	//- MRType.pre_text "Thinger"
	//-
	//- MContext.kind "CONTEXT"
	//- MContext.pre_text "test/iface"
	//- MOwner.pre_text "Thinger"
	//- MIdent.pre_text "Thing"
	Thing()
}
