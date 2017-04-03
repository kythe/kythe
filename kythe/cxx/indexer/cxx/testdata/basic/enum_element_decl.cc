// Checks that enumeration elements are modeled as constant members.
//- @E defines/binding EEnum
enum E {
//- @EM defines/binding EMElt
  EM
};
//- EMElt childof EEnum
//- EMElt.node/kind constant
//- EMElt.text 0
