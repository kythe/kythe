// Test two levels of compatibility_alias.

//- @Car defines/binding CarDecl
@interface Car
-(int) drive;
@end

//- @Car defines/binding CarImpl
@implementation Car
-(int) drive {
  return 20;
}
@end

//- @Truck defines/binding TruckAlias
//- @Car ref CarDecl
//- TruckAlias aliases CarDecl
@compatibility_alias Truck Car;

//- @Bike defines/binding BikeAlias
//- @Truck ref CarDecl
//- BikeAlias aliases CarDecl
@compatibility_alias Bike Truck;

int main(int argc, char **argv) {
  //- @bike defines/binding BikeVar
  //- BikeVar typed BikeAliasPtr
  //- BikeAliasPtr.node/kind TApp
  //- BikeAliasPtr param.0 vname("ptr#builtin", _, _, _, _)
  //- BikeAliasPtr param.1 CarImpl
  Bike *bike = [[Bike alloc] init];

  //- @car defines/binding CarVar
  //- CarVar typed CarPtrTy
  //- CarPtrTy.node/kind TApp
  //- CarPtrTy param.0 vname("ptr#builtin", _, _, _, _)
  //- CarPtrTy param.1 CarImpl
  Car *car = [[Car alloc] init];

  return 0;
}
