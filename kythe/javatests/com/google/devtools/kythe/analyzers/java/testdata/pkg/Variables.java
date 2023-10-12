package pkg;

// Check that fields/parameters/locals emit variable nodes

import java.io.IOException;
import java.io.OutputStream;

@SuppressWarnings("unused")
// - @Variables defines/binding Class
public class Variables {

  // - @field defines/binding V
  // - V.node/kind variable
  // - V.subkind field
  // - V childof Class
  // - V typed Str = vname(_,"jdk","","","java")
  // - Str.node/kind record
  // - Str.subkind class
  String field;

  // - @staticField defines/binding SF
  // - SF.node/kind variable
  // - SF.subkind field
  // - SF childof Class
  static int staticField;

  // - @m defines/binding F
  // - @p1 defines/binding P1
  // - @p2 defines/binding P2
  // - P1.node/kind variable
  // - P1.subkind local/parameter
  // - F param.0 P1
  // - F param.1 P2
  // - P1 typed Str
  // - P2 typed Int
  // - P1 childof F
  // - P2 childof F
  void m(String p1, int p2) throws IOException {

    // - @local defines/binding L
    // - L.node/kind variable
    // - L.subkind local
    // - L typed Int
    // - L childof F
    int local = 0;

    // - @stream defines/binding ResourceVar
    // - ResourceVar.node/kind variable
    // - ResourceVar.subkind local/resource
    // - ResourceVar childof F
    try (OutputStream stream = System.out) {
      // - @stream ref ResourceVar
      stream.write('\n');

      // - @#2io defines/binding ExceptionParam
      // - ExceptionParam.node/kind variable
      // - ExceptionParam.subkind local/exception
      // - ExceptionParam childof F
    } catch (java.io.IOException io) {
      // - @io ref ExceptionParam
      throw io;
    }
  }

  static {
    // - @localInStaticInitializer defines/binding StaticLocalVar
    // - StaticLocalVar.node/kind variable
    // - StaticLocalVar.subkind local
    // - StaticLocalVar childof Class
    int localInStaticInitializer = 0;
  }
}

// - !{V.tag/static _}
// - SF.tag/static _
