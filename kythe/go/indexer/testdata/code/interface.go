// Package iface tests code facts for an interface type.
//- @iface defines/binding Pkg
package iface

//- @Thinger defines/binding Thinger
//- Thinger code TName
//-
//- TName child.0 TContext
//- TContext.kind "CONTEXT"
//-
//- TName child.1 TIdent
//- TIdent.kind "IDENTIFIER"
//- TIdent.pre_text "Thinger"
//- TIdent link Thinger
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
	//- MName child.0 MContext
	//- MName child.1 MIdent
	//-
	//- MParams.kind "PARAMETER"
	//- MParams.pre_text "()"
	//-
	//- MRType.kind "TYPE"
	//- MRType.pre_text "Thinger"
	//-
	//- MContext.kind "CONTEXT"
	//- MContext.post_child_text "."
	//- MContext child.0 MPkg
	//- MPkg link Pkg
	//- MContext child.1 MOwner
	//- MOwner link Thinger
	//- MPkg.pre_text "test/iface"
	//- MOwner.pre_text "Thinger"
	//- MIdent.pre_text "Thing"
	//- MIdent link Thing
	Thing()
}
