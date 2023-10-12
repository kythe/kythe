package pkg.regression;

public class NullableRef {
  private NullableRef() {}

  static {
    // - @genericT ref GenericT
    TestNullable.genericT("");
  }
}
