package pkg;

@SuppressWarnings({"unused", "MultiVariableDeclaration"})
//- @Callgraph defines/binding Class
//- ClassInitDef.loc/start @^"Callgraph"
//- ClassInitDef.loc/end @^"Callgraph"
//- ClassInitDef defines ClassInit
public class Callgraph {

  // Implicit static class initializer
  //- ClassInit.node/kind function
  //- ClassInit childof Class
  //- !{ ClassInit.subkind "constructor" }

  //- NonStaticGCall.loc/start @^"g()"
  //- NonStaticGCall.loc/end @$"g()"
  //- NonStaticGCall ref/call G
  //- NonStaticGCall childof ECtor
  //- NonStaticGCall childof SCtor
  //- !{ NonStaticGCall childof Class }
  //- !{ NonStaticGCall childof ClassInit }
  final int zero = g();

  //- StaticGCall.loc/start @^"g()"
  //- StaticGCall.loc/end @$"g()"
  //- StaticGCall ref/call G
  //- StaticGCall childof ClassInit
  //- !{ StaticGCall childof ECtor }
  //- !{ StaticGCall childof SCtor }
  static final int ZERO_TWO = g();

  static {
    //- StaticBlockGCall.loc/start @^"g()"
    //- StaticBlockGCall.loc/end @$"g()"
    //- StaticBlockGCall ref/call G
    //- StaticBlockGCall childof ClassInit
    //- !{ StaticBlockGCall childof ECtor }
    //- !{ StaticBlockGCall childof SCtor }
    int zero = g();

    {
      //- NestedStaticBlockGCall.loc/start @^"g()"
      //- NestedStaticBlockGCall.loc/end @$"g()"
      //- NestedStaticBlockGCall ref/call G
      //- NestedStaticBlockGCall childof ClassInit
      //- NestedStaticBlockGCall childof ClassInit
      g();
    }
  }

  {
    //- BlockGCall.loc/start @^"g()"
    //- BlockGCall.loc/end @$"g()"
    //- BlockGCall ref/call G
    //- BlockGCall childof ECtor
    //- BlockGCall childof SCtor
    int zero = g();
  }

  //- CtorCall.loc/start @^"new Callgraph()"
  //- CtorCall.loc/end @$"new Callgraph()"
  //- CtorCall ref/call/direct ECtor
  //- CtorRef.loc/start @^"Callgraph"
  //- CtorRef.loc/end @$"Callgraph"
  //- CtorRef ref ECtor
  //- CtorCall childof ECtor
  //- CtorCall childof SCtor
  //- @Callgraph ref/id Class
  final Object instance = new Callgraph();

  //- @Callgraph defines/binding ECtor
  //- ECtor.node/kind function
  //- ECtor.subkind constructor
  public Callgraph() {}

  //- @Callgraph defines/binding SCtor
  //- SCtor.node/kind function
  //- SCtor.subkind constructor
  public Callgraph(String s) {}

  //- @f defines/binding F
  //- F.node/kind function
  //- F typed _FType
  static void f(int n) {
    //- @"Callgraph" ref ECtor
    //- @"Callgraph" ref/id Class
    //- ECtorCall.loc/start @^"new Callgraph()"
    //- ECtorCall.loc/end @$"new Callgraph()"
    Object cg = new Callgraph();
    //
    //- @"Callgraph" ref SCtor
    //- @"Callgraph" ref/id Class
    //- SCtorCall.loc/start @^"new Callgraph(null)"
    //- SCtorCall.loc/end @$"new Callgraph(null)"
    cg = new Callgraph(null);

    //- ECtorCall ref/call/direct ECtor
    //- ECtorCall childof F
    //- SCtorCall ref/call/direct SCtor
    //- SCtorCall childof F
  }

  //- @g defines/binding G
  //- G.node/kind function
  static int g() {
    //- CallAnchor.loc/start @^"f(4)"
    //- CallAnchor.loc/end   @$"f(4)"
    f(4);

    //- CallAnchor ref/call F
    //- CallAnchor childof  G
    return 0;
  }

  //- @Nested defines/binding NestedClass
  static class Nested {
    //- NestedInit.node/kind function
    //- NestedInit childof NestedClass
    //- !{ NestedInit.subkind "constructor" }

    //- ImplicitConstructor.node/kind function
    //- ImplicitConstructor.subkind "constructor"
    //- ImplicitConstructor childof NestedClass

    //- NestedClassCall.loc/start @^"g()"
    //- NestedClassCall.loc/end   @$"g()"
    //- NestedClassCall  childof  ImplicitConstructor
    final int someInt = g();

    static {
      //- NestedClassStaticCall.loc/start @^"g()"
      //- NestedClassStaticCall.loc/end   @$"g()"
      //- NestedClassStaticCall  childof  NestedInit
      g();
    }

    //- !{ NestedClassCall childof NestedInit
    //-    NestedClassStaticCall childof ImplicitConstructor }
  }
}
