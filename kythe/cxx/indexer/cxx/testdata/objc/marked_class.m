// Test marked source with attributes for classes.

//- @Data defines/binding DataFwdDecl
//- DataFwdDecl code CodeDataID
//- CodeDataID.kind "IDENTIFIER"
//- CodeDataID.pre_text "Data"
@class Data;

//- @Box defines/binding BoxIFace
//- BoxIFace code CodeIFaceID
//- CodeIFaceID.kind "IDENTIFIER"
//- CodeIFaceID.pre_text "Box"
@interface Box

@end

//- @Box defines/binding BoxImpl
//- BoxImpl code CodeImplID
//- CodeImplID.kind "IDENTIFIER"
//- CodeImplID.pre_text "Box"
@implementation Box

@end

//- @Package defines/binding PackageIFace
//- PackageIFace code CodePackageIFaceID
//- CodePackageIFaceID.kind "IDENTIFIER"
//- CodePackageIFaceID.pre_text "Package"
@interface Package : Box
@end
