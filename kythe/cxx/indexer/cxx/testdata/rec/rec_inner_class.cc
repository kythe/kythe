// Checks that inner classes are children of outer classes.
//- @C defines ClassC
//- !{ ClassC childof TheTranslationUnit }
class C {
  //- @R defines ClassR
  //- ClassR named vname("R:C#c",_,_,_,_)
  //- ClassR childof ClassC
  class R { };
};
