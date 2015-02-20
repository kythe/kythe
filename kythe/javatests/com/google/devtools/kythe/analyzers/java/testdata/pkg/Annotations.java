package pkg;

//- @Annotations defines Annotation
//- Annotation.node/kind interface
public @interface Annotations {}

//- @Annotations ref Annotation
//- @Deprecated ref Deprecated
@Annotations @Deprecated
//- @C defines C
//- C annotatedby Annotatedby
//- C annotatedby Deprecated
class C {

  //- @Deprecated ref Deprecated
  @Deprecated
  //- @field defines Field
  //- Field annotatedby Deprecated
  private String field;

  //- @Deprecated ref Deprecated
  //- @Override ref Override
  @Override @Deprecated
  //- @toString defines Method
  //- Method annotatedby Deprecated
  //- Method annotatedby Override
  public String toString() {
    return "C";
  }
}
