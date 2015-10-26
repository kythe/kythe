package pkg;

public class Snippets {

  public static void main(String[] args) {
    // Snippet over entire var decl
    //
    //- ADefAnchor defines/binding AVar
    //- ADefAnchor.loc/start @^a
    //- ADefAnchor.loc/end @$a
    //- ADefAnchor.snippet/start @^int
    //- ADefAnchor.snippet/end @$+3";"
    //- AnchorA ref AVar
    int a =
        10;

    //- @b defines/binding BVar
    //- AnchorB ref BVar
    int b = 12;

    // Snippet of entire statement
    //
    //- AnchorA.node/kind anchor
    //- AnchorA.loc/start @^a
    //- AnchorA.loc/end @$a
    //- AnchorB.node/kind anchor
    //- AnchorB.loc/start @^+7b
    //- AnchorB.loc/end @$+6b
    //- AnchorB.snippet/start @^System
    //- AnchorB.snippet/end @$+4";"
    //- AnchorA.snippet/start @^System
    //- AnchorA.snippet/end @$+2";"
    System.out.println(a +
                       b);
  }
}
