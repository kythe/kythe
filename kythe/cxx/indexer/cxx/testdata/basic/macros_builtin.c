// Tests that builtin macros are given stable vnames.
//- @"__STDC_VERSION__" ref/expands vname("__STDC_VERSION__#m@builtin",_,_,_,_)
int x = __STDC_VERSION__;
