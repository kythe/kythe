package pkg;

@SuppressWarnings("unused")
public class StaticVariables {
  //- @foo defines/binding StaticFoo
  private static String foo;

  //- @bar defines/binding StaticBar
  public static int bar;
}

//- StaticFoo.tag/static _
//- StaticBar.tag/static _
