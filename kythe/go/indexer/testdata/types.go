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

// float32 is not used in this package; it shouldn't be emitted
//- !{ _Float32Builtin=vname("float32#builtin", "golang.org", "", "", "go").node/kind tbuiltin }

//- EmptyTuple.node/kind tapp
//- EmptyTuple param.0 TupleBuiltin

//- @f0 defines/binding F0
//- F0 typed NullFuncType
//- NullFuncType.node/kind tapp
//- NullFuncType param.0 FnBuiltin
//- NullFuncType param.1 EmptyTuple
//- !{ NullFuncType param.2 _ }
func f0() {}

//- @f1 defines/binding F1
//- F1 typed F1FuncType
//- F1FuncType.node/kind tapp
//- F1FuncType param.0 FnBuiltin
//- F1FuncType param.1 EmptyTuple
//- F1FuncType param.2 IntBuiltin
//- F1FuncType param.3 BoolBuiltin
//- F1FuncType param.4 StringBuiltin
func f1(a int, b bool, c string) {}

//- @f2 defines/binding F2
//- F2 typed F2FuncType
//- F2FuncType.node/kind tapp
//- F2FuncType param.0 FnBuiltin
//- F2FuncType param.1 IntBuiltin
//- !{ NullFuncType param.2 _ }
func f2() int { return 0 }

//- @f3 defines/binding F3
//- F3 typed F3FuncType
//- F3FuncType.node/kind tapp
//- F3FuncType param.0 FnBuiltin
//- F3FuncType param.1 F3Return
//- F3Return.node/kind tapp
//- F3Return param.0 TupleBuiltin
//- F3Return param.1 IntBuiltin
//- F3Return param.2 BoolBuiltin
//- !{ NullFuncType param.2 _ }
func f3() (int, bool) { return 0, false }

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
	IntPointerField *int

	//- @IntArray4Field defines/binding IA4F
	//- IA4F typed IA4
	//- IA4.node/kind tapp
	//- IA4 param.0 Array4Builtin
	//- IA4 param.1 IntBuiltin
	IntArray4Field [4]int

	//- @IntSliceField defines/binding IntSliceField
	//- IntSliceField typed IntSlice
	//- IntSlice.node/kind tapp
	//- IntSlice param.0 SliceBuiltin
	//- IntSlice param.1 IntBuiltin
	IntSliceField []int

	//- @StrSetField defines/binding StrSetField
	//- StrSetField typed StrSet
	//- StrSet param.0 MapBuiltin
	//- StrSet param.1 StringBuiltin
	//- StrSet param.2 EmptyStruct
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
	整数型Chan chan int

	//- @RecvIntChan defines/binding RecvIntChanField
	//- RecvIntChanField typed RecvIntChan
	//- RecvIntChan.node/kind tapp
	//- RecvIntChan param.0 ChanRecvBuiltin
	//- RecvIntChan param.1 IntBuiltin
	RecvIntChan <-chan int

	//- @SendIntChan defines/binding SendIntChanField
	//- SendIntChanField typed SendIntChan
	//- SendIntChan.node/kind tapp
	//- SendIntChan param.0 ChanSendBuiltin
	//- SendIntChan param.1 IntBuiltin
	SendIntChan chan<- int
}

//- @sv defines/binding SVar
//- SVar.node/kind variable
//- SVar typed S
var sv = S{}

//- @Iter defines/binding Iter
//- Iter.node/kind interface
type Iter interface{}

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
