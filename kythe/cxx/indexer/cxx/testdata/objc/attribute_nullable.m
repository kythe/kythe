// Test that the nullable attribute is handled correctly.

//- @Data defines/binding DataDecl
@interface Data
@end

@interface Box

//- @cdata defines/binding CDataDecl
//- CDataDecl childof BoxDecl
//- @Data ref DataDecl
@property Data * _Nullable cdata;

@end

@implementation Box

@end

