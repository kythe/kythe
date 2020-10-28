package types

//- Array4Builtin=vname("array4#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- BoolBuiltin=vname("bool#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- ByteBuiltin=vname("byte#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- ChanBuiltin=vname("chan#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- ChanRecvBuiltin=vname("<-chan#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- ChanSendBuiltin=vname("chan<-#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- Float64Builtin=vname("float64#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- FnBuiltin=vname("fn#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- IntBuiltin=vname("int#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- MapBuiltin=vname("map#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- PointerBuiltin=vname("pointer#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- SliceBuiltin=vname("slice#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- StringBuiltin=vname("string#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- TupleBuiltin=vname("tuple#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- Uint8Builtin=vname("uint8#builtin", "golang.org", "", "", "go").node/kind tbuiltin
//- VariadicBuiltin=vname("variadic#builtin", "golang.org", "", "", "go").node/kind tbuiltin

// float32 is not used in this package; it shouldn't be emitted
//- !{ _Float32Builtin=vname("float32#builtin", "golang.org", "", "", "go").node/kind tbuiltin }

//- FnBuiltin code FnCode
//- FnCode.kind "TYPE"
//- FnCode.pre_text "fn"
//- BoolBuiltin code BoolCode
//- BoolCode.kind "TYPE"
//- BoolCode.pre_text "bool"

//- SliceTAppCode.kind "TYPE"
//- SliceTAppCode.pre_text "[]"
//- SliceTAppCode child.0 SliceTAppCodeParam
//- SliceTAppCodeParam.kind "LOOKUP_BY_PARAM"
//- SliceTAppCodeParam.lookup_index 1

//- PointerTAppCode.kind "TYPE"
//- PointerTAppCode.pre_text "*"
//- PointerTAppCode child.0 PointerTAppCodeParam
//- PointerTAppCodeParam.kind "LOOKUP_BY_PARAM"
//- PointerTAppCodeParam.lookup_index 1

//- TupleTAppCode.kind "TYPE"
//- TupleTAppCode.pre_text "("
//- TupleTAppCode.post_text ")"
//- TupleTAppCode child.0 TupleTAppCodeParam
//- TupleTAppCodeParam.kind "PARAMETER_LOOKUP_BY_PARAM"
//- TupleTAppCodeParam.post_child_text ", "
//- TupleTAppCodeParam.lookup_index 1

//- VariadicTAppCode.kind "TYPE"
//- VariadicTAppCode.pre_text "..."
//- VariadicTAppCode child.0 VariadicTAppCodeParam
//- VariadicTAppCodeParam.kind "LOOKUP_BY_PARAM"
//- VariadicTAppCodeParam.lookup_index 1

//- Array4TAppCode.kind "TYPE"
//- Array4TAppCode.pre_text "[4]"
//- Array4TAppCode child.0 Array4TAppCodeParam
//- Array4TAppCodeParam.kind "LOOKUP_BY_PARAM"
//- Array4TAppCodeParam.lookup_index 1

//- ChanTAppCode.kind "TYPE"
//- ChanTAppCode.pre_text "chan "
//- ChanTAppCode child.0 ChanTAppCodeParam
//- ChanTAppCodeParam.kind "LOOKUP_BY_PARAM"
//- ChanTAppCodeParam.lookup_index 1

//- SendChanTAppCode.kind "TYPE"
//- SendChanTAppCode.pre_text "chan<- "
//- SendChanTAppCode child.0 SendChanTAppCodeParam
//- SendChanTAppCodeParam.kind "LOOKUP_BY_PARAM"
//- SendChanTAppCodeParam.lookup_index 1

//- RecvChanTAppCode.kind "TYPE"
//- RecvChanTAppCode.pre_text "<-chan "
//- RecvChanTAppCode child.0 RecvChanTAppCodeParam
//- RecvChanTAppCodeParam.kind "LOOKUP_BY_PARAM"
//- RecvChanTAppCodeParam.lookup_index 1

//- MapTAppCode.kind "TYPE"
//- MapTAppCode.pre_text "map"
//- MapTAppCode child.0 MapTAppCodeKeyBox
//- MapTAppCodeKeyBox.kind "BOX"
//- MapTAppCodeKeyBox.pre_text "["
//- MapTAppCodeKeyBox.post_text "]"
//- MapTAppCodeKeyBox child.0 MapTAppCodeKey
//- MapTAppCodeKey.kind "LOOKUP_BY_PARAM"
//- MapTAppCodeKey.lookup_index 1
//- MapTAppCode child.1 MapTAppCodeValue
//- MapTAppCodeValue.kind "LOOKUP_BY_PARAM"
//- MapTAppCodeValue.lookup_index 2

//- FnTAppCode.kind "TYPE"
//- FnTAppCode child.0 FnTAppCodeParam
//- FnTAppCodeParam.kind "PARAMETER_LOOKUP_BY_PARAM"
//- FnTAppCodeParam.lookup_index 3
//- FnTAppCodeParam.pre_text "func("
//- FnTAppCodeParam.post_child_text ", "
//- FnTAppCodeParam.post_text ")"
//- FnTAppCode child.1 FnTAppCodeReturnBox
//- FnTAppCodeReturnBox.kind "BOX"
//- FnTAppCodeReturnBox.pre_text " "
//- FnTAppCodeReturnBox child.0 FnTAppCodeReturn
//- FnTAppCodeReturn.kind "LOOKUP_BY_PARAM"
//- FnTAppCodeReturn.lookup_index 1

//- VoidFnTAppCode.kind "TYPE"
//- VoidFnTAppCode child.0 VoidFnTAppCodeParam
//- VoidFnTAppCodeParam.kind "PARAMETER_LOOKUP_BY_PARAM"
//- VoidFnTAppCodeParam.lookup_index 3
//- VoidFnTAppCodeParam.pre_text "func("
//- VoidFnTAppCodeParam.post_child_text ", "
//- VoidFnTAppCodeParam.post_text ")"

//- MethodTAppCode.kind "TYPE"
//- MethodTAppCode child.0 MethodTAppCodeRecvBox
//- MethodTAppCodeRecvBox.kind "BOX"
//- MethodTAppCodeRecvBox.pre_text "("
//- MethodTAppCodeRecvBox.post_text ") "
//- MethodTAppCodeRecvBox child.0 MethodTAppCodeRecv
//- MethodTAppCodeRecv.kind "LOOKUP_BY_PARAM"
//- MethodTAppCodeRecv.lookup_index 2
//- MethodTAppCode child.1 MethodTAppCodeParam
//- MethodTAppCodeParam.kind "PARAMETER_LOOKUP_BY_PARAM"
//- MethodTAppCodeParam.lookup_index 3
//- MethodTAppCodeParam.pre_text "func("
//- MethodTAppCodeParam.post_child_text ", "
//- MethodTAppCodeParam.post_text ")"
//- MethodTAppCode child.2 MethodTAppCodeReturnBox
//- MethodTAppCodeReturnBox.kind "BOX"
//- MethodTAppCodeReturnBox.pre_text " "
//- MethodTAppCodeReturnBox child.0 MethodTAppCodeReturn
//- MethodTAppCodeReturn.kind "LOOKUP_BY_PARAM"
//- MethodTAppCodeReturn.lookup_index 1

//- VoidMethodTAppCode.kind "TYPE"
//- VoidMethodTAppCode child.0 VoidMethodTAppCodeRecvBox
//- VoidMethodTAppCodeRecvBox.kind "BOX"
//- VoidMethodTAppCodeRecvBox.pre_text "("
//- VoidMethodTAppCodeRecvBox.post_text ") "
//- VoidMethodTAppCodeRecvBox child.0 VoidMethodTAppCodeRecv
//- VoidMethodTAppCodeRecv.kind "LOOKUP_BY_PARAM"
//- VoidMethodTAppCodeRecv.lookup_index 2
//- VoidMethodTAppCode child.1 VoidMethodTAppCodeParam
//- VoidMethodTAppCodeParam.kind "PARAMETER_LOOKUP_BY_PARAM"
//- VoidMethodTAppCodeParam.lookup_index 3
//- VoidMethodTAppCodeParam.pre_text "func("
//- VoidMethodTAppCodeParam.post_child_text ", "
//- VoidMethodTAppCodeParam.post_text ")"

//- EmptyTuple.node/kind tapp
//- EmptyTuple param.0 TupleBuiltin
//- EmptyTuple code TupleTAppCode

//- @f0 defines/binding F0
//- F0 typed NullFuncType
//- NullFuncType.node/kind tapp
//- NullFuncType param.0 FnBuiltin
//- NullFuncType param.1 EmptyTuple
//- NullFuncType param.2 EmptyTuple
//- !{ NullFuncType param.3 _ }
//- NullFuncType code VoidFnTAppCode
func f0() {}

//- @f1 defines/binding F1
//- F1 typed F1FuncType
//- F1FuncType.node/kind tapp
//- F1FuncType param.0 FnBuiltin
//- F1FuncType param.1 EmptyTuple
//- F1FuncType param.2 EmptyTuple
//- F1FuncType param.3 IntBuiltin
//- F1FuncType param.4 BoolBuiltin
//- F1FuncType param.5 StringBuiltin
func f1(a int, b bool, c string) {}

//- @f2 defines/binding F2
//- F2 typed F2FuncType
//- F2FuncType.node/kind tapp
//- F2FuncType param.0 FnBuiltin
//- F2FuncType param.1 IntBuiltin
//- F2FuncType param.2 EmptyTuple
//- !{ NullFuncType param.3 _ }
//- F2FuncType code FnTAppCode
func f2() int { return 0 }

//- @f3 defines/binding F3
//- F3 typed F3FuncType
//- F3FuncType.node/kind tapp
//- F3FuncType param.0 FnBuiltin
//- F3FuncType param.1 F3Return
//- F3FuncType param.2 EmptyTuple
//- F3Return.node/kind tapp
//- F3Return param.0 TupleBuiltin
//- F3Return param.1 IntBuiltin
//- F3Return param.2 BoolBuiltin
//- !{ NullFuncType param.3 _ }
func f3() (int, bool) { return 0, false }

//- @f4 defines/binding F4
//- F4 typed F4FuncType
//- F4FuncType.node/kind tapp
//- F4FuncType param.0 FnBuiltin
//- F4FuncType param.1 EmptyTuple
//- F4FuncType param.2 EmptyTuple
//- F4FuncType param.3 IntBuiltin
//- F4FuncType param.4 VariadicInt
//- VariadicInt.node/kind tapp
//- VariadicInt param.0 VariadicBuiltin
//- VariadicInt param.1 IntBuiltin
//- VariadicInt code VariadicTAppCode
func f4(a int, b ...int) {}

func paramTypes(
	//- @intParam defines/binding IntParam
	//- IntParam typed IntBuiltin
	intParam int,
	//- @fParam defines/binding FParam
	//- FParam typed NullFuncType
	fParam func()) {
}

func retTypes() (
	//- @intRet defines/binding IntRet
	//- IntRet typed IntBuiltin
	intRet int,
	//- @fRet defines/binding FRet
	//- FRet typed NullFuncType
	fRet func()) {
	return 0, nil
}

//- @EmptyStruct defines/binding EmptyStruct
//- EmptyStruct typed EmptyStruct
type EmptyStruct struct{}

//- @S defines/binding S
//- S.node/kind record
type S struct {
	//- @Float64Field defines/binding Float64Field
	//- Float64Field.node/kind variable
	//- Float64Field typed Float64Builtin
	Float64Field float64

	//- @IntPointerField defines/binding IntPointerField
	//- IntPointerField.node/kind variable
	//- IntPointerField typed IntPointer
	//- IntPointer.node/kind tapp
	//- IntPointer param.0 PointerBuiltin
	//- IntPointer param.1 IntBuiltin
	//- IntPointer code PointerTAppCode
	IntPointerField *int

	//- @IntArray4Field defines/binding IA4F
	//- IA4F typed IA4
	//- IA4.node/kind tapp
	//- IA4 param.0 Array4Builtin
	//- IA4 param.1 IntBuiltin
	//- IA4 code Array4TAppCode
	IntArray4Field [4]int

	//- @IntSliceField defines/binding IntSliceField
	//- IntSliceField typed IntSlice
	//- IntSlice.node/kind tapp
	//- IntSlice param.0 SliceBuiltin
	//- IntSlice param.1 IntBuiltin
	//- IntSlice code SliceTAppCode
	IntSliceField []int

	//- @StrSetField defines/binding StrSetField
	//- StrSetField typed StrSet
	//- StrSet param.0 MapBuiltin
	//- StrSet param.1 StringBuiltin
	//- StrSet param.2 EmptyStruct
	//- StrSet code MapTAppCode
	StrSetField map[string]EmptyStruct

	//- @ByteField defines/binding ByteField
	//- ByteField typed ByteBuiltin
	ByteField byte

	//- @Uint8Field defines/binding Uint8Field
	//- Uint8Field typed Uint8Builtin
	Uint8Field uint8

	//- @"整数型Chan" defines/binding IntChanField
	//- IntChanField.node/kind variable
	//- IntChanField typed IntChan
	//- IntChan.node/kind tapp
	//- IntChan param.0 ChanBuiltin
	//- IntChan param.1 IntBuiltin
	//- IntChan code ChanTAppCode
	整数型Chan chan int

	//- @RecvIntChan defines/binding RecvIntChanField
	//- RecvIntChanField typed RecvIntChan
	//- RecvIntChan.node/kind tapp
	//- RecvIntChan param.0 ChanRecvBuiltin
	//- RecvIntChan param.1 IntBuiltin
	//- RecvIntChan code RecvChanTAppCode
	RecvIntChan <-chan int

	//- @SendIntChan defines/binding SendIntChanField
	//- SendIntChanField typed SendIntChan
	//- SendIntChan.node/kind tapp
	//- SendIntChan param.0 ChanSendBuiltin
	//- SendIntChan param.1 IntBuiltin
	//- SendIntChan code SendChanTAppCode
	SendIntChan chan<- int
}

//- @sv defines/binding SVar
//- SVar.node/kind variable
//- SVar typed S
var sv = S{}

//- @Method defines/binding Method
//- Method typed MethodType
//- MethodType.node/kind tapp
//- MethodType param.0 FnBuiltin
//- MethodType param.1 IntBuiltin
//- MethodType param.2 S
//- MethodType code MethodTAppCode
func (s S) Method() int { return 0 }

//- @PMethod defines/binding PMethod
//- PMethod typed PMethodType
//- PMethodType.node/kind tapp
//- PMethodType param.0 FnBuiltin
//- PMethodType param.1 EmptyTuple
//- PMethodType param.2 SPointer
//- SPointer.node/kind tapp
//- SPointer param.0 PointerBuiltin
//- SPointer param.1 S
//- MethodType code VoidMethodTAppCode
func (s *S) PMethod() {}

//- @Iter defines/binding Iter
//- Iter.node/kind interface
type Iter interface {
	//- @Method defines/binding IMethod
	//- IMethod typed IMethodType
	//- IMethodType.node/kind tapp
	//- IMethodType param.0 FnBuiltin
	//- IMethodType param.1 IntBuiltin
	//- IMethodType param.2 Iter
	Method() int
}

//- @iv defines/binding IVar
//- IVar.node/kind variable
//- IVar typed Iter
var iv Iter

//- @main defines/binding Main
//- Main typed NullFuncType
func main() {
	//- @i defines/binding LocalAssign
	//- LocalAssign.node/kind variable
	//- LocalAssign typed IntBuiltin
	i := 0

	//- @localF defines/binding LocalF
	//- LocalF.node/kind variable
	//- LocalF typed LocalFType
	//- LocalFType.node/kind tapp
	//- LocalFType param.0 FnBuiltin
	localF := func(a int) { print(a) }

	localF(i)
}

// TODO(schroederc): taliases
//- @StringAlias defines/binding StringAlias
//- StringAlias.node/kind record
//- StringAlias typed StringBuiltin
type StringAlias = string
