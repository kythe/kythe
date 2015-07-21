// Checks implicit-style (plain BCPL) juxtaposed documentation.

enum class C {
  E1,  // doc1
  E2   // doc2
};

//- Etor1 named vname("E1:C#n",_,_,_,_)
//- Etor2 named vname("E2:C#n",_,_,_,_)
//- Doc1 documents Etor1
//- Doc2 documents Etor2
//- Doc1.loc/start 87
//- Doc1.loc/end 94
//- Doc2.loc/start 102
//- Doc2.loc/end 109
