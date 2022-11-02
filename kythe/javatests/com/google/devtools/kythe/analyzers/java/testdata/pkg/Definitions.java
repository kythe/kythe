package pkg;

//- DefinitionsDef defines Definitions
//- DefinitionsDef.node/kind anchor
//- DefinitionsDef.loc/start @^"@Suppress"
@SuppressWarnings("unused")
//- @Definitions defines/binding Definitions
public class Definitions {

  //- @HELLO defines/binding HelloConstant
  //- HelloDef=vname(_,_,_,_,"java") defines HelloConstant
  //- HelloDef.node/kind anchor
  //- HelloDef.loc/start @^private
  //- HelloDef.loc/end @$";"
  private static final String HELLO = "こんにちは";

  //- @"HELLO2" defines/binding Hello2
  //- Hello2Def defines Hello2
  //- Hello2Def.node/kind anchor
  //- Hello2Def.loc/start @^private
  //- Hello2Def.loc/end @$";"
  private static final String HELLO2 = "今日は";  // こんにちは

  //- @Inner defines/binding Inner
  //- InnerClassDef defines Inner
  //- InnerClassDef.node/kind anchor
  //- InnerClassDef.loc/start @^static
  static class Inner {
    //- InnerClassDef.loc/end @$"}"
  }

  //- @param defines/binding ParamVar
  //- @"int param" defines ParamVar
  private static void f(int param) {}

  //- @main defines/binding MainMethod
  //- MainDef defines MainMethod
  //- MainDef.node/kind anchor
  //- MainDef.loc/start @^public
  public static void main(String[] args) {
    //- @name defines/binding NameVar
    //- NameDef defines NameVar
    //- NameDef.node/kind anchor
    //- NameDef.loc/start @^String
    //- NameDef.loc/end @$";"
    String name = "Kythe";

    //- @punctuation defines/binding PuncVar
    //- PuncDef defines PuncVar
    //- PuncDef.node/kind anchor
    //- PuncDef.loc/start @^String
    //- PuncDef.loc/end @$";"
    String punctuation;

    //- @punctuation ref/writes PuncVar
    punctuation = "!";

    System.out.printf("%s %s%s\n", HELLO, name, punctuation);
    //- MainDef.loc/end @$"}"
  }

  //- DefinitionsDef.loc/end @$"}"
}
