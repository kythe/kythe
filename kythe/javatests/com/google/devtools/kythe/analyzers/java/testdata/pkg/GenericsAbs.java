package pkg;

@SuppressWarnings("unused")
//- @GenericsAbs=vname(_, Corpus, Root, Path, _) defines/binding GAbs
//- Class childof GAbs=vname(_, Corpus, Root, Path, _)
//- GAbs.node/kind abs
//- GAbs param.0 TVar
//- @T defines/binding TVar
//- TVar.node/kind absvar
public class GenericsAbs<T> {
  //- !{ _ tparam.0 _ }
  //- !{ _.node/kind tvar }

  //- @obj defines/binding V
  //- V typed Obj
  Object obj;

  //- @print defines/binding PrintMethod
  //- @print defines/binding PrintAbs
  //- PrintMethod childof PrintAbs
  //- PrintAbs.node/kind abs
  //- @P defines/binding PVar
  //- PrintAbs param.0 PVar
  //- PVar.node/kind absvar
  //- PVar bounded/upper.0 Obj
  public static <P> void print(
      //- @P ref PVar
      P p) {
    System.out.println(p.toString());
  }

  //- @T ref TVar
  public void g(T t) {}

  public static void f() {
    //- @"GenericsAbs<String>" ref GType
    //- @gs defines/binding GVar
    //- GVar typed GType
    //- GType.node/kind tapp
    //- GType param.0 GAbs
    //- GType param.1 _Str
    GenericsAbs<String> gs =
        //- @"GenericsAbs<String>" ref GType
        new GenericsAbs<String>();

    //- @"GenericsAbs" ref Class
    //- @nonGeneric defines/binding NGVar
    //- NGVar typed NGType
    //- NGType.node/kind record
    GenericsAbs nonGeneric =
        //- @"GenericsAbs" ref Class
        new GenericsAbs();

    //- @"Optional<GenericsAbs<String>>" ref OType
    //- OType.node/kind tapp
    //- OType param.0 _Optional
    //- OType param.1 GType
    //- @opt defines/binding OVar
    //- OVar typed OType
    Optional<GenericsAbs<String>> opt;
  }

  //- @Optional defines/binding OptionalAbs
  //- _OptionalClass childof OptionalAbs
  //- @T defines/binding OptionalTVar
  //- OptionalTVar.node/kind absvar
  private static class Optional<T> {}

  //- @U defines/binding UVar
  //- UVar.node/kind absvar
  //- @List ref List
  //- @Inter ref Inter
  //- UVar bounded/upper.0 List
  //- UVar bounded/upper.1 Inter
  private static class Bounded<U extends java.util.List & Inter> {}

  //- @classTypeVarBound defines/binding _ClassTypeVarBoundFunc
  //- @E defines/binding EVar
  //- EVar bounded/upper.0 TVar
  public <E extends T> void classTypeVarBound() {}

  public <
          //- @X defines/binding XVar
          X,
          //- @Y defines/binding YVar
          //- YVar bounded/upper.0 XVar
          Y extends X>
      //- @ownTypeVarBound defines/binding _OwnTypeVarBoundFunc
      void ownTypeVarBound() {}

  // Verify that, if there are interface bounds, then java.lang.Object appears as a superclass bound
  // (and in the type parameter's signature) iff it's provided explicitly.
  // This matters because a bound of Object&Iface is actually different from a bound of just Iface:
  // The former's erasure is Object, the latter's is Iface, and erasure is a language-level concept
  // (see e.g., https://docs.oracle.com/javase/specs/jls/se8/html/jls-4.html#jls-4.6)

  // We test the cases of single vs. multiple interface bounds because those have different code
  // paths
  // (the latter is implemented by javac as an intersection type).

  // With only an (implicit or explicit) bound of Object, do emit a bounded/upper.0 edge,
  // but don't add the superfluous "extends java.lang.Object" in the name.

  //- @noIFaceBound defines/binding _Func
  //- @S0 defines/binding S0Var
  //- S0Var bounded/upper.0 Obj
  public <S0> void noIFaceBound() {}

  //- @objAndNoIFaceBound defines/binding _OFunc
  //- @S1 defines/binding S1Var
  //- S1Var bounded/upper.0 Obj
  public <S1> void objAndNoIFaceBound() {}

  // If there is at least one interface bound, only emit a bound of java.lang.Object if it was
  // explicit.

  //- @oneIFaceBound defines/binding _IFunc
  //- @S2 defines/binding S2Var
  //- @List ref List
  //- !{ S2Var bounded/upper.0 Obj }
  //- S2Var bounded/upper.0 List
  public <S2 extends java.util.List> void oneIFaceBound() {}

  //- @objAndOneIFaceBound defines/binding _OIFunc
  //- @S3 defines/binding S3Var
  //- @List ref List
  //- S3Var bounded/upper.0 Obj
  //- S3Var bounded/upper.1 List
  public <S3 extends Object & java.util.List> void objAndOneIFaceBound() {}

  //- @twoIfaceBounds defines/binding _IIFunc
  //- @S4 defines/binding S4Var
  //- @List ref List
  //- @Inter ref Inter
  //- !{ S4Var bounded/upper.0 Obj }
  //- S4Var bounded/upper.0 List
  //- S4Var bounded/upper.1 Inter
  public <S4 extends java.util.List & Inter> void twoIfaceBounds() {}

  //- @objAndTwoIFaceBounds defines/binding _OIIFunc
  //- @S5 defines/binding S5Var
  //- @List ref List
  //- @Inter ref Inter
  //- S5Var bounded/upper.0 Obj
  //- S5Var bounded/upper.1 List
  //- S5Var bounded/upper.2 Inter
  public <S5 extends Object & java.util.List & Inter> void objAndTwoIFaceBounds() {}

  //- @Inter defines/binding Inter
  private static interface Inter {}
}
