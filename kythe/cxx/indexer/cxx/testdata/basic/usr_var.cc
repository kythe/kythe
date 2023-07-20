// We index USRs for variables.

//- @global defines/binding Global
//- GlobalUSR=vname(_,"",_,_,_) /clang/usr Global
//- GlobalUSR.node/kind clang/usr
int global;

//- @param defines/binding Param
//- !{_ /clang/usr Param}
void f(int param) { }

void g() {
//- @local defines/binding Local
//- !{_ /clang/usr Local} 
  int local;
}
