package com.google.devtools.kythe.analyzers.java.testdata.pkg;

@SuppressWarnings("unused")
//- CA=@Scopes defines/binding ScopesClass
//- CA childof File
//- File.node/kind file
public class Scopes {

  //- @Scopes childof ScopesClass
  public Scopes() {}

  //- IA=@Inner defines/binding InnerClass
  //- IA childof ScopesClass
  static class Inner {
    //- @innerMethod childof InnerClass
    static void innerMethod() {}
  }

  //- MA=@method defines/binding Method
  //- MA childof ScopesClass
  static Object method(int param) {
    //- @var childof Method
    int var = 1;

    //- @var childof Method
    //- @param childof Method
    var += param;

    //- LA=@LocalClass defines/binding LocalClass
    //- LA childof Method
    //- LocalClass childof Method
    class LocalClass {
      //- @field childof LocalClass
      int field;
    }

    //- L2A=@LocalClass childof Method
    //- L2A ref/id LocalClass
    return new LocalClass();
  }
}
