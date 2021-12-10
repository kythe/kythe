// Test that we handle restricted type arguments correctly when that
// restriction is based on a protocol.

@interface O
@end

@implementation O
@end

//- @Proto defines/binding ProtoDecl
@protocol Proto
@end

@implementation ObjParent
@end

@interface ObjChild : ObjParent<Proto>
@end

@implementation ObjChild
@end


// No source range defines BoxDecl since this is a generic type.
//- @Type defines/binding TypeVar
//- @Box defines/binding BoxDecl
//- TypeVar.node/kind tvar
//- BoxDecl tparam.0 TypeVar
//- TypeVar bounded/upper ProtoDeclType
//- ProtoDeclType.node/kind tapp
//- ProtoDeclType param.0 vname("ptr#builtin", _, _, _, _)
//- ProtoDeclType param.1 ProtoDecl
//- @Proto ref ProtoDecl
@interface Box<Type:id<Proto> > : O
-(int) addToList:(Type)item;
@end

@implementation Box
-(int) addToList:(id)item {
  return 1;
}
@end

int main(int argc, char **argv) {
  Box<ObjChild *> *b = [[Box alloc] init];
  ObjChild *o = [[ObjChild alloc] init];
  [b addToList:o];
  return 0;
}
