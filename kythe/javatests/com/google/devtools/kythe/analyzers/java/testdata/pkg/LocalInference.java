package pkg;

import java.util.List;

final class LocalInference {



  public static void testLocalTypeInference() {
    //- @"inferred" typed IntType
    var inferred = 1;
    //- @"specified" typed IntType
    int specified = 2;
  }

  public static void testLoopTypeInference() {
    //- @"n" typed IntegerType
    for (var n : List.of(1,2,3)) {
      //- @"v" typed IntegerType
      Integer v = n;
    }
  }

  public static void testLambdaTypeInference() {
    Callable lambda = (var i) -> i + 1;
  }

  private static interface Callable {
    int run(int i);
  }
}
