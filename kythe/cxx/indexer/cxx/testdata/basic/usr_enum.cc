// We index USRs for enums.

//- @E defines/binding E
//- EUsr /clang/usr E
//- EUsr.node/kind clang/usr
enum E {
//- @Etor defines/binding Etor
//- EtorUsr /clang/usr Etor
//- EtorUsr.node/kind clang/usr
  Etor
};
