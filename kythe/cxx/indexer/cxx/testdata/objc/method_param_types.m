// Simple test to make sure parameter types ref their definitions.

//- @O1 defines/binding O1Decl
@interface O1
@end

//- @O1 defines/binding O1Impl
@implementation O1
@end

//- @O2 defines/binding O2Decl
@interface O2
@end

//- @O2 defines/binding O2Impl
@implementation O2
@end

@interface Box
//- @O1 ref O1Impl
//- @O2 ref O2Impl
-(int) foo:(O1*)p1 withL:(O2*)p2;

//- @O1 ref O1Impl
//- @O2 ref O2Impl
-(int) bar:(O1*)p1 withL:(O2*)p2;
@end

@implementation Box
//- @O1 ref O1Impl
//- @O2 ref O2Impl
-(int) foo:(O1*)p1 withL:(O2*)p2 {
  return 0;
}

//- @O1 ref O1Impl
//- @O2 ref O2Impl
-(int) bar:(O1*)p1 withL:(O2*)p2 {
  return 0;
}
@end
