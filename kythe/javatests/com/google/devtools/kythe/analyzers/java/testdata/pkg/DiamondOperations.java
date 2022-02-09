package pkg;

final class DiamondOperations {
  private static void testAnonymousInnerDiamond() {
    //- @"Generic<String>" ref GenericStringType
    //- GenericStringType.node/kind tapp
    //- GenericStringType param.0 GenericType
    Generic<String> iter =
        //- @"Generic<>" ref/id DiamondType
        //- DiamondType.node/kind tapp
        //- DiamondType param.0 GenericType
        new Generic<>() {
          @Override
          public String method() {
            return "";
          }
        };
  }

  private static void testTrivialDiamondOperator() {
    // TODO(#3745): Change this test so that GenericIntType and DiamondIntType are the same.
    //- @"Generic<Integer>" ref GenericIntType
    //- GenericIntType.node/kind tapp
    //- GenericIntType param.0 GenericType
    //- @"Generic<>" ref/id DiamondIntType
    //- DiamondIntType.node/kind tapp
    //- DiamondIntType param.0 GenericType
    Generic<Integer> n = new Generic<>();
  }

  private static class Generic<T> {
    public T method() {
      return null;
    }
  }
}
