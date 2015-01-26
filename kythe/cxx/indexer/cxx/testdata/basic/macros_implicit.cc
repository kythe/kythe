// Checks that we can handle implicit macros.
//- @"__LINE__" ref/expands vname("__LINE__#m@invalid",_,_,_,_)
int x = __LINE__;
//- @"__FILE__" ref/expands vname("__FILE__#m@invalid",_,_,_,_)
const char *f = __FILE__;
//- @"__COUNTER__" ref/expands vname("__COUNTER__#m@invalid",_,_,_,_)
int y = __COUNTER__;
