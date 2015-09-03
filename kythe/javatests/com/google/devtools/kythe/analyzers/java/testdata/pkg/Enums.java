package pkg;

//- @Enums defines/binding Enum
//- Enum.node/kind sum
//- Enum.subkind enumClass
public enum Enums {

  //- @A defines/binding A
  //- A.node/kind constant
  //- A childof Enum
  //- @B defines/binding B
  //- B childof Enum
  //- B.node/kind constant
  A, B,

  //- @C defines/binding C
  //- C childof Enum
  //- C.node/kind constant
  C("hello");

  private Enums(String s) {}
  private Enums() {}
}
