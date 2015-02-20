package pkg;

//- @Generics defines Class
//- @Generics defines GAbs
//- Class childof GAbs
//- GAbs.node/kind abs
//- GAbs param.0 TVar
//- @T defines TVar
//- TVar.node/kind absvar
public class Generics<T> {

  //- @print defines PrintMethod
  //- @print defines PrintAbs
  //- PrintMethod childof PrintAbs
  //- PrintAbs.node/kind abs
  //- @P defines PVar
  //- PrintAbs param.0 PVar
  //- PVar.node/kind absvar
  public static <P> void print(
      //- @P ref PVar
      P p) {
    System.out.println(p.toString());
  }

  //- @T ref TVar
  public void g(T t) {}

  public static void f() {
    //- @"Generics<String>" ref GType
    //- @gs defines GVar
    //- GVar typed GType
    //- GType.node/kind tapp
    //- GType param.0 Class
    //- GType param.1 Str
    Generics<String> gs =
        //- @"Generics<String>" ref GType
        new Generics<String>();

    //- @"Optional<Generics<String>>" ref OType
    //- OType.node/kind tapp
    //- OType param.0 Optional
    //- OType param.1 GType
    //- @opt defines OVar
    //- OVar typed OType
    Optional<Generics<String>> opt;
  }

  //- @"Optional<?>" ref OptionalWild
  //- OptionalWild.node/kind tapp
  //- OptionalWild param.0 OptionalClass
  //- OptionalWild param.1 Wildcard0
  //- Wildcard0.node/kind absvar
  //- OptionalWild named vname("pkg.Generics.Optional<?>","","","","java")
  //- !{ Wildcard0 bounded/upper Anything0
  //-    Wildcard0 bounded/lower Anything1 }
  private static void wildcard(Optional<?> o) {}

  //- @"Optional<? extends String>" ref OptionalWildString
  //- OptionalWildString.node/kind tapp
  //- OptionalWildString param.0 OptionalClass
  //- OptionalWildString param.1 Wildcard1
  //- Wildcard1.node/kind absvar
  //- Wildcard1 bounded/upper Str
  //- @String ref Str
  //- !{ Wildcard1 bounded/lower Anything2 }
  //- OptionalWildString named vname("pkg.Generics.Optional<? extends java.lang.String>","","","","java")
  private static void wildcardBound(Optional<? extends String> o) {}

  //- @Optional defines OptionalClass
  //- @T defines OptionalTVar
  //- OptionalTVar.node/kind absvar
  private static class Optional<T> {}

  //- @BV defines BVar
  //- BVar.node/kind absvar
  //- @List ref List
  //- @Inter ref Inter
  //- BV bounded/upper List
  //- BV bounded/upper Inter
  private static class Bounded<BV extends java.util.List & Inter> {}

  //- @Inter defines Inter
  private static interface Inter {}
}
