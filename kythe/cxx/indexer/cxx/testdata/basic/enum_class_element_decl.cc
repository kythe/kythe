// Checks that enumeration class elements are modeled as constant members.
//- @E defines/binding EEnum
enum class E {
//- @EM defines/binding EMElt
  EM
};
//- EMElt childof EEnum
//- EMElt.node/kind constant
//- EMElt.text 0
