// Test marked source with attributes.

@class Data;

@interface Box

//- @fooP1 defines/binding FooArg1
//- FooArg1 code ACRoot
//- ACRoot child.0 ACType
//- ACType.kind "TYPE"
//- ACType.pre_text "Data * _Nullable"
//- ACRoot child.1 ACSpace
//- ACRoot child.2 ACIdent
//- ACIdent child.0 ACContext
//- ACContext.kind "CONTEXT"
//- ACContext child.0 ACContextIdent
//- ACContextIdent.kind "IDENTIFIER"
//- ACIdent child.1 ACIdentToken
//- ACIdentToken.kind "IDENTIFIER"
//- ACIdentToken.pre_text "fooP1"
- (int) foofunc:(Data * _Nullable)fooP1;

@end

@implementation Box

//- @fooP1 defines/binding FooDArg1
//- FooDArg1 code ACDRoot
//- ACDRoot child.0 ACDType
//- ACDType.kind "TYPE"
//- ACDType.pre_text "Data * _Nullable"
//- ACDRoot child.1 ACDSpace
//- ACDRoot child.2 ACDIdent
//- ACDIdent child.0 ACDContext
//- ACDContext.kind "CONTEXT"
//- ACDContext child.0 ACDContextIdent
//- ACDContextIdent.kind "IDENTIFIER"
//- ACDIdent child.1 ACDIdentToken
//- ACDIdentToken.kind "IDENTIFIER"
//- ACDIdentToken.pre_text "fooP1"
- (int) foofunc:(Data * _Nullable)fooP1 {
  return 0;
}

@end
