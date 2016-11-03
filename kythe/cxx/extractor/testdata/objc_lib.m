// Checks that Objective-C classes are declared and defined.

// Not needed but tests that we are able to correctly find it
#import <Availability.h>

#import <Foundation/Foundation.h>

//- @Box defines/binding BoxIface
//- BoxIface.node/kind record
//- BoxIface.subkind class
//- BoxIface.complete incomplete
@interface Box : NSObject
@end

//- @Box defines/binding BoxImpl
//- BoxImpl.node/kind record
//- BoxImpl.subkind class
//- BoxImpl.complete definition
@implementation Box
@end

//- BoxIface named BoxName
//- BoxImpl named BoxName

int main(int argc, char **argv) {
  return 0;
}

