// Checks implicit-style (plain BCPL) juxtaposed documentation.

//- Etor1 named vname("E1:C#n",_,_,_,_)
//- Etor2 named vname("E2:C#n",_,_,_,_)
//- @:9"// doc1" documents Etor1
//- @:10"// doc2" documents Etor2

enum class C {
  E1,  // doc1
  E2   // doc2
};
