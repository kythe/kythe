// Checks implicit-style (plain BCPL) multiline juxtaposed documentation.

enum class C {
  E1,  // doc11
       // doc12
  E2X   // doc2
        // doc22
};

//- Etor1 named vname("E1:C#n",_,_,_,_)
//- Etor2 named vname("E2X:C#n",_,_,_,_)
//- Doc1 documents Etor1
//- Doc2 documents Etor2
//- Doc1.loc/start 97
//- Doc1.loc/end 121
//- Doc2.loc/start 130
//- Doc2.loc/end 154
