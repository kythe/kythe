namespace nasa {
//- @Shuttle defines/binding StructS
struct Shuttle {
  //- @Launch defines/binding MemLaunch
  void Launch();
  //- @Launch defines/binding MemLaunchInt
  void Launch(int);
};
//- @Launch defines/binding FnLaunch
void Launch();
//- @Launch defines/binding FnLaunchInt
void Launch(int);
}

namespace nasb {
//- @Shuttle ref StructS
using nasa::Shuttle;
//- @Launch ref FnLaunch
//- @Launch ref FnLaunchInt
using nasa::Launch;

//- @Discovery defines/binding StructD
struct Discovery : Shuttle {
  //- @Launch ref MemLaunch
  //- @Launch ref MemLaunchInt
  using Shuttle::Launch;
};

void fn() {
  //- @Shuttle ref StructS
  Shuttle s;
  //- @Discovery ref StructD
  Discovery d;
  //- @Launch ref MemLaunch
  d.Launch();
}
}
