package pkg;

//- @Enums defines Enum
//- Enum.node/kind sum
//- Enum.subkind enumClass
public enum Enums {

  //- @A defines A
  //- A.node/kind constant
  //- A childof Enum
  //- @B defines B
  //- B childof Enum
  //- B.node/kind constant
  A, B,

  //- @C defines C
  //- C childof Enum
  //- C.node/kind constant
  C("hello");

  private Enums(String s) {}
  private Enums() {}
}
