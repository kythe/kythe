// Test that compatibility_alias defines a type alias. compatibility_alias is
// very similar to typedef.
//
// Clang does not keep track of the aliases, so it is not trivial to determine
// if something is using an alias or the actual type. This affects us because
// our type graph will skip the alias and point directly to the aliased type.

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
  int i = [truck drive];

  return 0;
}
