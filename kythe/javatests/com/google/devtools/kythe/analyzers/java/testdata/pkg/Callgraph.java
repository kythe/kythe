package pkg;

public class Callgraph {

  //- @f defines/binding F
  //- F.node/kind function
  //- F typed FType
  static void f(int n) {}

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
