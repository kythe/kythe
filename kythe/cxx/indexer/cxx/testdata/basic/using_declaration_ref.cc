namespace nasa {
//- @Shuttle defines/binding StructS
struct Shuttle {
  //- @Shuttle defines/binding ShuttleCtor
  explicit Shuttle(int);

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
  //- @#1Shuttle ref ShuttleCtor
  using Shuttle::Shuttle;

  //- @Launch ref MemLaunch
  //- @Launch ref MemLaunchInt
  using Shuttle::Launch;
};

struct Endeavour : Discovery {
  //- @#1Discovery ref ShuttleCtor
  using Discovery::Discovery;
};

void fn() {
  //- @Shuttle ref StructS
  //- @s ref ShuttleCtor
  //- @"s(1)" ref/call ShuttleCtor
  Shuttle s(1);
  //- @Discovery ref StructD
  //- @d ref ShuttleCtor
  //- @"d(1)" ref/call ShuttleCtor
  Discovery d(1);
  //- @Launch ref MemLaunch
  d.Launch();

  //- @end ref ShuttleCtor
  //- @"end(1)" ref/call ShuttleCtor
  Endeavour end(1);
}
}
