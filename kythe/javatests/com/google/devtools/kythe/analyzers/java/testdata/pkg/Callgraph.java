package pkg;

//- @Callgraph defines/binding Class
public class Callgraph {

  // Implicit static class initializer
  //- ClassInit.node/kind function
  //- ClassInit childof Class
  //- !{ ClassInit.subkind "constructor" }

  //- StaticGCall.loc/start @^"g()"
  //- StaticGCall.loc/end @$"g()"
  //- StaticGCall ref/call G
  //- StaticGCall childof ECtor
  //- StaticGCall childof SCtor
  //- !{ StaticGCall childof Class }
  //- !{ StaticGCall childof ClassInit }
  final int ZERO = g();

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
  //- CtorCall ref/call ECtor
  //- CtorCall childof ECtor
  //- CtorCall childof SCtor
  final Callgraph INSTANCE = new Callgraph();

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
    //- @"new Callgraph" ref ECtor
    //- ECtorCall.loc/start @^"new Callgraph()"
    //- ECtorCall.loc/end @$"new Callgraph()"
    Callgraph cg = new Callgraph();
    //
    //- @"new Callgraph" ref SCtor
    //- SCtorCall.loc/start @^"new Callgraph(null)"
    //- SCtorCall.loc/end @$"new Callgraph(null)"
    cg = new Callgraph(null);

    //- ECtorCall ref/call ECtor
    //- ECtorCall childof F
    //- SCtorCall ref/call SCtor
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
    final int INT = g();

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
