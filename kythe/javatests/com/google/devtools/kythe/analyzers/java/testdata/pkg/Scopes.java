package com.google.devtools.kythe.analyzers.java.testdata.pkg;

@SuppressWarnings("unused")
//- @Scopes defines/binding ScopesClass
public class Scopes {

  //- @Scopes childof ScopesClass
  public Scopes() {}

  //- @Inner defines/binding InnerClass
  //- @Inner childof ScopesClass
  static class Inner {
    //- @innerMethod childof InnerClass
    static void innerMethod() {}
  }

  //- @method defines/binding Method
  //- @method childof ScopesClass
  static Object method(int param) {
    //- @var childof Method
    int var = 1;

    //- @var childof Method
    //- @param childof Method
    var += param;

    //- @LocalClass defines/binding LocalClass
    //- @LocalClass childof Method
    //- LocalClass childof Method
    class LocalClass {
      //- @field childof LocalClass
      int field;
    }

    //- @LocalClass childof Method
    return new LocalClass();
  }
}
