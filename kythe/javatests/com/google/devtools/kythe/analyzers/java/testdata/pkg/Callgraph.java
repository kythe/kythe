package pkg;

public class Callgraph {

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
  //- F typed FType
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
  static void g() {
    //- CallAnchor.loc/start @^"f(4)"
    //- CallAnchor.loc/end   @$"f(4)"
    f(4);

    //- CallAnchor ref/call F
    //- CallAnchor childof  G
  }
}
