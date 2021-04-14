package pkg;

@SuppressWarnings("unused")
public class StaticVariables {
  //- @foo defines/binding StaticFoo
  private static String foo;

  //- @bar defines/binding StaticBar
  public static int bar;

  //- @E defines/binding Enum
  enum E {
    //- @X defines/binding EnumValue
    X,
    Y;
  }
}

//- StaticFoo.tag/static _
//- StaticBar.tag/static _
//- !{ Enum.tag/static _ }
//- EnumValue.tag/static _
