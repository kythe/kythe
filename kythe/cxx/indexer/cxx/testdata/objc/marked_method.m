// Test marked source with attributes for methods.

@class Data;

@interface Box

//- @foofunc defines/binding FooFuncDecl
//- FooFuncDecl code FCDeclRoot
//- FCDeclRoot child.0 FCDeclInt
//- FCDeclInt.kind "TYPE"
//- FCDeclInt.pre_text int
//- FCDeclRoot child.1 FCDeclParen
//- FCDeclParen.pre_text ") "
//- FCDeclRoot child.2 FCDeclIdentRoot
//- FCDeclIdentRoot child.0 _
//- FCDeclIdentRoot child.1 FCDeclIdent
//- FCDeclIdent.pre_text "foofunc:"
//- @fooP1 defines/binding FooDeclArg1
//- FooDeclArg1 code ACDeclRoot
//- ACDeclRoot child.0 ACDeclType
//- ACDeclType.kind "TYPE"
//- ACDeclType.pre_text "Data *"
//- ACDeclRoot child.1 _
//- ACDeclRoot child.2 ACDeclIdent
//- ACDeclIdent child.0 ACDeclContext
//- ACDeclContext.kind "CONTEXT"
//- ACDeclContext child.0 ACDeclContextIdent
//- ACDeclContextIdent.kind "IDENTIFIER"
//- ACDeclContextIdent.pre_text "Box"
//- ACDeclContext child.1 ACDeclContextIdentName
//- ACDeclContextIdentName.kind "IDENTIFIER"
//- ACDeclContextIdentName.pre_text "foofunc:"
//- ACDeclIdent child.1 ACDeclIdentToken
//- ACDeclIdentToken.kind "IDENTIFIER"
//- ACDeclIdentToken.pre_text "fooP1"
- (int) foofunc:(Data *)fooP1;

//- @barfunc defines/binding BarFuncDecl
//- BarFuncDecl code BFCDeclRoot
//- BFCDeclRoot child.0 BFCDeclInt
//- BFCDeclInt.kind "TYPE"
//- BFCDeclInt.pre_text int
//- BFCDeclRoot child.1 BFCDeclParen
//- BFCDeclParen.pre_text ") "
//- BFCDeclRoot child.2 BFCDeclIdentRoot
//- BFCDeclIdentRoot child.0 _
//- BFCDeclIdentRoot child.1 BFCDeclIdent
//- BFCDeclIdent.pre_text "barfunc:moreargs:"
//- @fooP1 defines/binding BarDeclArg1
//- BarDeclArg1 code BACDeclRoot
//- BACDeclRoot child.0 BACDeclType
//- BACDeclType.kind "TYPE"
//- BACDeclType.pre_text "Data *"
//- BACDeclRoot child.1 _
//- BACDeclRoot child.2 BACDeclIdent
//- BACDeclIdent child.0 BACDeclContext
//- BACDeclContext.kind "CONTEXT"
//- BACDeclContext child.0 BACDeclContextIdent
//- BACDeclContextIdent.kind "IDENTIFIER"
//- BACDeclContextIdent.pre_text "Box"
//- BACDeclContext child.1 BACDeclContextIdentName
//- BACDeclContextIdentName.kind "IDENTIFIER"
//- BACDeclContextIdentName.pre_text "barfunc:moreargs:"
//- BACDeclIdent child.1 BACDeclIdentToken
//- BACDeclIdentToken.kind "IDENTIFIER"
//- BACDeclIdentToken.pre_text "fooP1"
//- @arg2 defines/binding BarDeclArg2
//- BarDeclArg2 code BA2CDeclRoot
//- BA2CDeclRoot child.0 BA2CDeclType
//- BA2CDeclType.kind "TYPE"
//- BA2CDeclType.pre_text "int"
//- BA2CDeclRoot child.1 _
//- BA2CDeclRoot child.2 BA2CDeclIdent
//- BA2CDeclIdent child.0 BA2CDeclContext
//- BA2CDeclContext.kind "CONTEXT"
//- BA2CDeclContext child.0 BA2CDeclContextIdent
//- BA2CDeclContextIdent.kind "IDENTIFIER"
//- BA2CDeclContextIdent.pre_text "Box"
//- BA2CDeclContext child.1 BA2CDeclContextIdentName
//- BA2CDeclContextIdentName.kind "IDENTIFIER"
//- BA2CDeclContextIdentName.pre_text "barfunc:moreargs:"
//- BA2CDeclIdent child.1 BA2CDeclIdentToken
//- BA2CDeclIdentToken.kind "IDENTIFIER"
//- BA2CDeclIdentToken.pre_text "arg2"
- (int) barfunc:(Data *)fooP1 moreargs:(int)arg2;

//- @noargs defines/binding NoArgsFuncDecl
//- NoArgsFuncDecl code NFCDeclRoot
//- NFCDeclRoot child.0 NFCDeclInt
//- NFCDeclInt.kind "TYPE"
//- NFCDeclInt.pre_text int
//- NFCDeclRoot child.1 NFCDeclParen
//- NFCDeclParen.pre_text ") "
//- NFCDeclRoot child.2 NFCDeclIdentRoot
//- NFCDeclIdentRoot child.0 _
//- NFCDeclIdentRoot child.1 NFCDeclIdent
//- NFCDeclIdent.pre_text "noargs"
- (int) noargs;

@end

@implementation Box

//- @foofunc defines/binding FooFuncDefn
//- FooFuncDefn code FCDefnRoot
//- FCDefnRoot child.0 FCDefnInt
//- FCDefnInt.kind "TYPE"
//- FCDefnInt.pre_text int
//- FCDefnRoot child.1 FCDefnParen
//- FCDefnParen.pre_text ") "
//- FCDefnRoot child.2 FCDefnIdentRoot
//- FCDefnIdentRoot child.0 _
//- FCDefnIdentRoot child.1 FCDefnIdent
//- FCDefnIdent.pre_text "foofunc:"
//- @fooP1 defines/binding FooDArg1
//- FooDArg1 code ACDefnRoot
//- ACDefnRoot child.0 ACDefnType
//- ACDefnType.kind "TYPE"
//- ACDefnType.pre_text "Data *"
//- ACDefnRoot child.1 _
//- ACDefnRoot child.2 ACDefnIdent
//- ACDefnIdent child.0 ACDefnContext
//- ACDefnContext.kind "CONTEXT"
//- ACDefnContext child.0 ACDefnContextIdent
//- ACDefnContextIdent.kind "IDENTIFIER"
//- ACDefnContextIdent.pre_text "Box"
//- ACDefnContext child.1 ACDefnContextIdentName
//- ACDefnContextIdentName.kind "IDENTIFIER"
//- ACDefnContextIdentName.pre_text "foofunc:"
//- ACDefnIdent child.1 ACDefnIdentToken
//- ACDefnIdentToken.kind "IDENTIFIER"
//- ACDefnIdentToken.pre_text "fooP1"
- (int) foofunc:(Data *)fooP1 {
  return 0;
}

//- @barfunc defines/binding BarFuncDefn
//- BarFuncDefn code BFCDefnRoot
//- BFCDefnRoot child.0 BFCDefnInt
//- BFCDefnInt.kind "TYPE"
//- BFCDefnInt.pre_text int
//- BFCDefnRoot child.1 BFCDefnParen
//- BFCDefnParen.pre_text ") "
//- BFCDefnRoot child.2 BFCDefnIdentRoot
//- BFCDefnIdentRoot child.0 _
//- BFCDefnIdentRoot child.1 BFCDefnIdent
//- BFCDefnIdent.pre_text "barfunc:moreargs:"
//- @fooP1 defines/binding BarDefnArg1
//- BarDefnArg1 code BACDefnRoot
//- BACDefnRoot child.0 BACDefnType
//- BACDefnType.kind "TYPE"
//- BACDefnType.pre_text "Data *"
//- BACDefnRoot child.1 _
//- BACDefnRoot child.2 BACDefnIdent
//- BACDefnIdent child.0 BACDefnContext
//- BACDefnContext.kind "CONTEXT"
//- BACDefnContext child.0 BACDefnContextIdent
//- BACDefnContextIdent.kind "IDENTIFIER"
//- BACDefnContextIdent.pre_text "Box"
//- BACDefnContext child.1 BACDefnContextIdentName
//- BACDefnContextIdentName.kind "IDENTIFIER"
//- BACDefnContextIdentName.pre_text "barfunc:moreargs:"
//- BACDefnIdent child.1 BACDefnIdentToken
//- BACDefnIdentToken.kind "IDENTIFIER"
//- BACDefnIdentToken.pre_text "fooP1"
//- @arg2 defines/binding BarDefnArg2
//- BarDefnArg2 code BA2CDefnRoot
//- BA2CDefnRoot child.0 BA2CDefnType
//- BA2CDefnType.kind "TYPE"
//- BA2CDefnType.pre_text "int"
//- BA2CDefnRoot child.1 _
//- BA2CDefnRoot child.2 BA2CDefnIdent
//- BA2CDefnIdent child.0 BA2CDefnContext
//- BA2CDefnContext.kind "CONTEXT"
//- BA2CDefnContext child.0 BA2CDefnContextIdent
//- BA2CDefnContextIdent.kind "IDENTIFIER"
//- BA2CDefnContextIdent.pre_text "Box"
//- BA2CDefnContext child.1 BA2CDefnContextIdentName
//- BA2CDefnContextIdentName.kind "IDENTIFIER"
//- BA2CDefnContextIdentName.pre_text "barfunc:moreargs:"
//- BA2CDefnIdent child.1 BA2CDefnIdentToken
//- BA2CDefnIdentToken.kind "IDENTIFIER"
//- BA2CDefnIdentToken.pre_text "arg2"
- (int) barfunc:(Data *)fooP1 moreargs:(int)arg2 {
  return 0;
}

//- @noargs defines/binding NoArgsFuncDefn
//- NoArgsFuncDefn code NFCDefnRoot
//- NFCDefnRoot child.0 NFCDefnInt
//- NFCDefnInt.kind "TYPE"
//- NFCDefnInt.pre_text int
//- NFCDefnRoot child.1 NFCDefnParen
//- NFCDefnParen.pre_text ") "
//- NFCDefnRoot child.2 NFCDefnIdentRoot
//- NFCDefnIdentRoot child.0 _
//- NFCDefnIdentRoot child.1 NFCDefnIdent
//- NFCDefnIdent.pre_text "noargs"
- (int) noargs {
  return 0;
}

@end
