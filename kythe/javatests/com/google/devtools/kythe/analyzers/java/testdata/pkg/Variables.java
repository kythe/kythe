package pkg;

// Check that fields/parameters/locals emit variable nodes

//- @Variables defines Class
public class Variables {

  //- @Deprecated ref Deprecated
  @Deprecated
  //- @field defines V
  //- V.node/kind variable
  //- V childof Class
  //- V typed Str = vname("java.lang.String","","","","java")
  //- Str.node/kind name
  String field;

  //- @m defines F
  //- @p1 defines P1
  //- @p2 defines P2
  //- F param.0 P1
  //- F param.1 P2
  //- P1 typed Str
  //- P2 typed Int
  void m(String p1, int p2) {

    //- @local defines L
    //- L.node/kind variable
    //- L typed Int
    int local = 0;
  }
}
