package pkg;

public class Callgraph {

  //- @f defines/binding F
  //- F.node/kind function
  //- F typed FType
  static void f(int n) {}

  //- F callableas FCA
  //- FCA.node/kind callable
  //- FCA typed FType

  //- @g defines/binding G
  //- G.node/kind function
  static void g() {
    //- CallAnchor.loc/start @^"f(4)"
    //- CallAnchor.loc/end   @$"f(4)"
    f(4);

    //- CallAnchor ref/call FCA
    //- CallAnchor childof  G
  }

  //- G callableas GCA
  //- GCA.node/kind callable
}
