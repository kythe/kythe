// Test that the indexer correctly handles compatibility_alias in the presence
// of a macro and extra whitespace.

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

#define Supercar Car
#define Monstertruck BigTruck

//- @"Truck" defines/binding TruckAlias
//- @"Car" ref CarDecl
//- TruckAlias aliases CarDecl
@compatibility_alias Truck     Car;

//- @Truck2 defines/binding Truck2Alias
//- @Supercar ref CarDecl
//- Truck2Alias aliases CarDecl
@compatibility_alias Truck2 Supercar;

//- @Monstertruck defines/binding MTruckAlias
//- @Car ref CarDecl
//- MTruckAlias aliases CarDecl
@compatibility_alias Monstertruck Car;

int main(int argc, char **argv) {
  //- @truck defines/binding TruckVar
  //- TruckVar typed PtrTruckAlias
  //- PtrTruckAlias.node/kind TApp
  //- PtrTruckAlias param.0 vname("ptr#builtin", _, _, _, _)
  //- PtrTruckAlias param.1 CarImpl
  Truck *truck = [[Truck alloc] init];

  //- @car defines/binding CarVar
  //- CarVar typed CarPtrTy
  //- CarPtrTy.node/kind TApp
  //- CarPtrTy param.0 vname("ptr#builtin", _, _, _, _)
  //- CarPtrTy param.1 CarImpl
  Car *car = [[Car alloc] init];

  //- @truck2 defines/binding Truck2Var
  //- Truck2Var typed PtrTruckAlias
  Truck2 *truck2 = [[Truck2 alloc] init];

  //- @mtruck defines/binding MTruckVar
  //- MTruckVar typed PtrTruckAlias
  Monstertruck *mtruck = [[Monstertruck alloc] init];

  return 0;
}
