package pkg;

//- @Callgraph defines/binding Class
public class Callgraph {

  //- StaticGCall.loc/start @^"g()"
  //- StaticGCall.loc/end @$"g()"
  //- StaticGCall ref/call G
  //- StaticGCall childof Class
  final int ZERO = g();

  static {
    //- StaticBlockGCall.loc/start @^"g()"
    //- StaticBlockGCall.loc/end @$"g()"
    //- StaticBlockGCall ref/call G
    //- StaticBlockGCall childof Class
    int zero = g();
  }

  {
    //- BlockGCall.loc/start @^"g()"
    //- BlockGCall.loc/end @$"g()"
    //- BlockGCall ref/call G
    //- BlockGCall childof Class
    int zero = g();
  }

  //- StaticCtorCall.loc/start @^"new Callgraph()"
  //- StaticCtorCall.loc/end @$"new Callgraph()"
  //- StaticCtorCall ref/call ECtor
  //- StaticCtorCall childof Class
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
}
