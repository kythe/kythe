// Checks that inner defns are children of outer classes.
template <typename T>
//- @C defines/binding AbsC
//- ClassC childof AbsC
class C {
  //- @R defines/binding EnumR
  //- EnumR childof ClassC
  enum R : T {
  };
};
//- @cshort defines/binding CShort
//- CShort typed CShortAbs
//- CShortAbsInst specializes CShortAbs
//- CShortAbsEnum childof CShortAbsInst
//- CShortAbsEnum.node/kind sum
//- !{ CShortAbsEnum childof ClassC }
C<short> cshort;
