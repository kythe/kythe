// Multiline documentation can be placed next to documented elements.
enum class C {
///<- @Cr defines EnumeratorCr
///<- JxDoc documents EnumeratorCr
///<- JxDoc.loc/start @^"///"
  Cr ///< an
     ///<- JxDoc.loc/end @$tor
     ///< enumerator
};
//- goal_prefix should be ///<-
