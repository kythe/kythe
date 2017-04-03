package pkg;

// Check that fields/parameters/locals emit variable nodes

import java.io.IOException;
import java.io.OutputStream;

//- @Variables defines/binding Class
public class Variables {

  //- @field defines/binding V
  //- V.node/kind variable
  //- V.subkind field
  //- V childof Class
  //- V typed Str = vname(_,"jdk","","","java")
  //- Str.node/kind record
  //- Str.subkind class
  String field;

  //- @m defines/binding F
  //- @p1 defines/binding P1
  //- @p2 defines/binding P2
  //- P1.node/kind variable
  //- P1.subkind local/parameter
  //- F param.0 P1
  //- F param.1 P2
  //- P1 typed Str
  //- P2 typed Int
  void m(String p1, int p2) throws IOException {

    //- @local defines/binding L
    //- L.node/kind variable
    //- L.subkind local
    //- L typed Int
    int local = 0;

    //- @stream defines/binding ResourceVar
    //- ResourceVar.node/kind variable
    //- ResourceVar.subkind local/resource
    try (OutputStream stream = System.out) {
      //- @stream ref ResourceVar
      stream.write('\n');

      //- @ioe defines/binding ExceptionParam
      //- ExceptionParam.node/kind variable
      //- ExceptionParam.subkind local/exception
    } catch (IOException ioe) {
      //- @ioe ref ExceptionParam
      throw ioe;
    }
  }
}
