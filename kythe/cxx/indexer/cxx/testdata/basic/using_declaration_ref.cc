namespace nasa {
//- @Shuttle defines/binding StructS
struct Shuttle {};
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

void fn() {
  // @Shuttle ref StructS
  Shuttle s;
}
}
