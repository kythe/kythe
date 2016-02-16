package pkg;

//- @Interfaces defines/binding Inter
//- Inter.node/kind interface
public interface Interfaces {
  //- @sanityCheck defines/binding SanityCheck
  //- SanityCheck childof Inter
  public static <T> boolean sanityCheck(Ordered<T> ordering, T x, T y) {
    return (ordering.equalTo(x, y) && ordering.equalTo(y, x))
            || (ordering.lessThan(x, y) && ordering.moreThan(y, x))
            || (ordering.lessThan(y, x) && ordering.moreThan(x, y));
  }

  //- @Ordered defines/binding OrderedInterface
  //- @"Ordered<T>" defines/binding OrderedTAbs
  //- @Interfaces ref Inter
  //- OrderedInterface extends Inter
  public static interface Ordered<T> extends Interfaces {
    //- @lessThan defines/binding OrderedLessThan
    //- OrderedLessThan childof OrderedInterface
    boolean lessThan(T x, T y);

    //- @equalTo defines/binding OrderedEqualTo
    //- OrderedEqualTo childof OrderedInterface
    default boolean equalTo(T x, T y) {
      //- @lessThan ref OrderedLessThan
      //- @moreThan ref OrderedMoreThan
      return !lessThan(x, y) && !moreThan(x, y);
    }

    //- @moreThan defines/binding OrderedMoreThan
    //- OrderedMoreThan childof OrderedInterface
    default boolean moreThan(T x, T y) {
      //- @lessThan ref OrderedLessThan
      return lessThan(y, x);
    }
  }

  //- @Deredro defines/binding DeredroInterface
  //- @"Deredro<T>" defines/binding DeredroTAbs
  //- @T defines/binding TypeVariable
  //- DeredroTAbs param.0 TypeVariable
  public static interface Deredro<T>
      //- @Ordered ref OrderedInterface
      //- @T ref TypeVariable
      //- OrderedTApp.node/kind tapp
      //- OrderedTApp param.0 OrderedInterface
      //- OrderedTApp param.1 TypeVariable
      //- DeredroInterface extends OrderedTApp
      extends Ordered<T> {
    //- @lessThan defines/binding DeredroLessThan
    //- DeredroLessThan overrides OrderedLessThan
    //- DeredroLessThan childof DeredroInterface
    default boolean lessThan(T x, T y) {
      //- @moreThan ref DeredroMoreThan
      return moreThan(y, x);
    }

    //- @moreThan defines/binding DeredroMoreThan
    //- DeredroMoreThan overrides OrderedMoreThan
    //- DeredroMoreThan childof DeredroInterface
    boolean moreThan(T x, T y);
  }

  //- @IntComparison defines/binding IntComparisonClass
  //- @Integer ref IntegerClass
  //- DeredroTApp.node/kind tapp
  //- DeredroTApp param.0 DeredroInterface
  //- DeredroTApp param.1 IntegerClass
  //- IntComparisonClass extends DeredroTApp
  public static class IntComparison implements Deredro<Integer> {
    //- @lessThan defines/binding IntegerLessThan
    //- IntegerLessThan overrides DeredroLessThan
    //- IntegerLessThan childof IntComparisonClass
    @Override public boolean lessThan(Integer x, Integer y) {
      return x < y;
    }

    // TODO(mazurak) "overrides/transitive" is what the Java indexer currently outputs here, but
    // maybe it shouldn't. We need a more rigorous definition for the various "overrides" edges.
    //
    //- @equalTo defines/binding IntegerEqualTo
    //- IntegerEqualTo overrides/transitive OrderedEqualTo
    //- IntegerEqualTo childof IntComparisonClass
    @Override public boolean equalTo(Integer x, Integer y) {
      return x.equals(y);
    }

    //- @moreThan defines/binding IntegerMoreThan
    //- IntegerMoreThan overrides DeredroMoreThan
    //- IntegerMoreThan childof IntComparisonClass
    @Override public boolean moreThan(Integer x, Integer y) {
      return x > y;
    }
  }

  //- @Further defines/binding FurtherClass
  //- @IntComparison ref IntComparisonClass
  //- FurtherClass extends IntComparisonClass
  public static class Further extends IntComparison {
    //- @equalTo defines/binding FurtherEqualTo
    //- FurtherEqualTo overrides IntegerEqualTo
    //- FurtherEqualTo childof FurtherClass
    @Override public boolean equalTo(Integer x, Integer y) {
      return y.equals(x);
    }
  }

  // TODO(mazurak): should @FunctionalInterface classes interact with calleables in some way?
  //- @FunctionalInterface ref FunctionalAnnotation
  //- @Reducer defines/binding ReducerInterface
  //- ReducerInterface annotatedby FunctionalAnnotation
  @FunctionalInterface public static interface Reducer<T, S> {
    //- @reduce defines/binding ReduceMethod
    //- ReduceMethod childof ReducerInterface
    S reduce(T x, T y);
  }
}
