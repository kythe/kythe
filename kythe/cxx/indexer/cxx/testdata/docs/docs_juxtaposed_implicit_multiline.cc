// Checks implicit-style (plain BCPL) multiline juxtaposed documentation.

//- Etor1 named vname("E1:C#n",_,_,_,_)
//- Etor2 named vname("E2X:C#n",_,_,_,_)
//- Doc1 documents Etor1
//- Doc2 documents Etor2
//- Doc1.loc/start @^:13"// doc11"
//- Doc1.loc/end @$:14"12"
//- Doc2.loc/start @^:15"// doc2"
//- Doc2.loc/end @$:16"22"

enum class C {
  E1,  // doc11
       // doc12
  E2X   // doc2
        // doc22
};
