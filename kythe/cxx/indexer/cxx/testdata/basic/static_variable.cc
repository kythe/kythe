// Tests that static variables are properly tagged.


class C {
  //- @f defines/binding StaticFieldF
  static int f;
  //- @b defines/binding StaticFieldB
  static char b;
  //- @c defines/binding NonStaticFieldC
  int c;
};

//- StaticFieldF.tag/static _
//- StaticFieldB.tag/static _
//- !{ NonStaticFieldC.tag/static _ }
