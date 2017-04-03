// Checks that inner classes are children of outer classes.
//- @C defines/binding ClassC
//- !{ ClassC childof TheTranslationUnit }
class C {
  //- @R defines/binding ClassR
  //- ClassR childof ClassC
  class R { };
};
