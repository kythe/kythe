// Checks implicit-style (plain BCPL) multiline juxtaposed documentation.

//- Doc1 documents Etor1
//- Doc2 documents Etor2
//- Doc1.loc/start @^:11"// doc11"
//- Doc1.loc/end @$:12"12"
//- Doc2.loc/start @^:13"// doc2"
//- Doc2.loc/end @$:14"22"

enum class C {
  E1,  // doc11
       // doc12
  E2X   // doc2
        // doc22
};
