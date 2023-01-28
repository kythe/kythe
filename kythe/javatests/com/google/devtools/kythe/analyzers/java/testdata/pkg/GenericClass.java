package pkg;

@SuppressWarnings("unused")
//- @GenericClass defines/binding GAbs
//- Class childof GAbs
//- GAbs.node/kind abs
//- GAbs param.0 TVar
//- @T defines/binding TVar
//- TVar.node/kind tparam
public class GenericClass<T> {

  public static void foo() {
    //- @GenericClass ref GAbs
    //- @String ref StringType
    //- @var defines/binding VarVar
    //- VarVar typed OType
    //- OType.node/kind tapp
    //- OType tparam.0 GAbs
    //- OType tparam.1 StringType
    GenericClass<String> var;
  }
}
