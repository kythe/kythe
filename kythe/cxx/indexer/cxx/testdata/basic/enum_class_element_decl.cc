// Checks that enumeration class elements are modeled as constant members.
//- @E defines EEnum
enum class E {
//- @EM defines EMElt
  EM
};
//- EMElt childof EEnum
//- EMElt.node/kind constant
//- EMElt.text 0
//- EMElt named vname("EM:E#n",_,_,_,_)