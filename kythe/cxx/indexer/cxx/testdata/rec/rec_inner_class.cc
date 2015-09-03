// Checks that inner classes are children of outer classes.
//- @C defines/binding ClassC
//- !{ ClassC childof TheTranslationUnit }
class C {
  //- @R defines/binding ClassR
  //- ClassR named vname("R:C#c",_,_,_,_)
  //- ClassR childof ClassC
  class R { };
};
