package pkg;

public class Selectors {
  //- @field defines Field
  String field;

  //- @Optional ref Optional
  //- @maybe defines Param
  public String m(Optional<String> maybe) {
    //- @maybe ref Param
    //- @isPresent ref IsPresentMethod
    if (maybe.isPresent()) {
      //- @maybe ref Param
      //- @get ref GetMethod
      //- @field ref Field
      //- @this ref ThisM
      this.field = maybe.get();
    }
    //- @this ref This
    //- @m2 ref M2Method
    //- @toString ref ToStringMethod
    return this.m2().toString();
  }

  //- @m2 defines M2Method
  private String m2() {
    //- @field ref Field
    //- @this ref This
    return this.field;
  }

  //- @Optional defines Optional
  private static interface Optional<T> {
    public T get();
    public boolean isPresent();
  }
}
