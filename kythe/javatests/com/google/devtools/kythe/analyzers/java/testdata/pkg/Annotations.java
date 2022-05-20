package pkg;

//- @Annotations defines/binding Annotation
//- Annotation.node/kind interface
public @interface Annotations {
  //- @classes defines/binding ClassesM
  Class<?>[] classes() default {};

  //- @String ref StringClass
  Class<?> withDef() default String.class;
}

@SuppressWarnings("unused")
//- @Annotations ref/id Annotation
@Annotations(
  //- @C ref C
  //- @Annotations ref Annotation
  //- @classes ref ClassesM
  classes = {C.class, Annotations.class}
)
//- @Deprecated ref/id Deprecated
@Deprecated
//- @C defines/binding C
//- C annotatedby Deprecated
class C {

  //- @Deprecated ref/id Deprecated
  @Deprecated
  //- @field defines/binding Field
  //- Field annotatedby Deprecated
  //- @String ref StringClass
  private String field;

  //- @Deprecated ref/id Deprecated
  //- @Override ref/id Override
  @Override @Deprecated
  //- @toString defines/binding Method
  //- Method annotatedby Deprecated
  //- Method annotatedby Override
  public String toString() {
    return "C";
  }
}
