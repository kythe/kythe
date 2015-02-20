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
