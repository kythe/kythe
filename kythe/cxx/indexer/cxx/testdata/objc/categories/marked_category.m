// Test marked source with attributes for category methods.
@class Data;

@interface Base
@end

@implementation Base
@end

@interface Base (Cat)

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
//- ACDeclContextIdent.pre_text "Base(Cat)"
//- ACDeclContext child.1 ACDeclContextIdentName
//- ACDeclContextIdentName.kind "IDENTIFIER"
//- ACDeclContextIdentName.pre_text "foofunc:"
//- ACDeclIdent child.1 ACDeclIdentToken
//- ACDeclIdentToken.kind "IDENTIFIER"
//- ACDeclIdentToken.pre_text "fooP1"
- (int) foofunc:(Data *)fooP1;

@end

@implementation Base (Cat)

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
//- ACDefnContextIdent.pre_text "Base(Cat)"
//- ACDefnContext child.1 ACDefnContextIdentName
//- ACDefnContextIdentName.kind "IDENTIFIER"
//- ACDefnContextIdentName.pre_text "foofunc:"
//- ACDefnIdent child.1 ACDefnIdentToken
//- ACDefnIdentToken.kind "IDENTIFIER"
//- ACDefnIdentToken.pre_text "fooP1"
- (int) foofunc:(Data *)fooP1 {
  return 20;
}

@end
