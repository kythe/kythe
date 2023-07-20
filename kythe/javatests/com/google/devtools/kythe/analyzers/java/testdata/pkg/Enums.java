package pkg;

@SuppressWarnings("unused")
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
  //- @A ref EnumCtor
  //- @A ref/call/direct EnumCtor
  //- @B ref EnumCtor
  //- @B ref/call/direct EnumCtor
  A, B,

  //- @C defines/binding C
  //- C childof Enum
  //- C.node/kind constant
  //- @C ref EnumStrCtor
  //- @"C(\"hello\")" ref/call/direct EnumStrCtor
  C("hello"),

  //- @D defines/binding D
  //- D childof Enum
  //- D.node/kind constant
  D {
  },

  //- @E defines/binding E
  //- E.node/kind constant
  //- @E ref EnumCtr
  //- @"E()" ref/call/direct EnumCtr
  E();

  //- @Enums defines/binding EnumStrCtor
  private Enums(String s) {}
  //- @Enums defines/binding EnumCtor
  private Enums() {}
}
