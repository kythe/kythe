// Test what compatibility_alias does in the presence of a typedef.
//
// It turns out that the alias is replaced by the root type, so in the
// following example, the alias Truck is replaced with Car, which skips
// through the intermediate alias of SUV.

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

//- @SUV defines/binding SUVTy
typedef Car SUV;

//- @Truck defines/binding TruckAlias
//- @SUV ref CarDecl
//- TruckAlias aliases CarDecl
@compatibility_alias Truck SUV;

int main(int argc, char **argv) {
  //- @truck defines/binding TruckVar
  //- TruckVar typed PtrTruckAlias
  //- PtrTruckAlias.node/kind TApp
  //- PtrTruckAlias param.0 vname("ptr#builtin", _, _, _, _)
  //- PtrTruckAlias param.1 CarImpl
  Truck *truck = [[Truck alloc] init];

  //- @suv defines/binding SuvVar
  //- SuvVar typed PtrSuvAlias
  //- PtrSuvAlias.node/kind TApp
  //- PtrSuvAlias param.0 vname("ptr#builtin", _, _, _, _)
  //- PtrSuvAlias param.1 SUVTy
  SUV *suv = [[SUV alloc] init];

  //- @car defines/binding CarVar
  //- CarVar typed CarPtrTy
  //- CarPtrTy.node/kind TApp
  //- CarPtrTy param.0 vname("ptr#builtin", _, _, _, _)
  //- CarPtrTy param.1 CarImpl
  Car *car = [[Car alloc] init];
  int i = [truck drive];

  return 0;
}
