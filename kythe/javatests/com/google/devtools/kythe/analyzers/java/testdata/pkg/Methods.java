package pkg;

//- @IOException ref/imports IOE
import java.io.IOException;

//- @Methods defines/binding Class
public class Methods {

  //- @Methods defines/binding Ctor
  //- Ctor.node/kind function
  //- Ctor childof Class
  //- Ctor typed CtorType
  //- CtorType param.0 FnBuiltin
  //- CtorType param.1 Class
  public Methods() {}

  //- @Methods ref Class
  public static Methods create() {
    //- @Methods ref Class
    //- @"new Methods" ref Ctor
    return new Methods();
  }

  //- @noop defines/binding NoOp
  //- NoOp typed NullFunc
  //- NullFunc param.0 FnBuiltin = vname("fn#builtin", "", "", "", "java")
  //- NullFunc param.1 VoidBuiltin = vname("void#builtin", "", "", "", "java")
  private void noop() {}

  //- @f defines/binding F
  //- F.node/kind function
  //- F childof Class
  public static int f(int a) {
    return 0;
  }

  //- @f2 defines/binding F2
  //- F2.node/kind function
  //- F2 childof Class
  //- F2 typed StrFunc
  private String f2() {
    return null;
  }

  //- @f3 defines/binding F3
  //- F3 typed StrFunc
  //- StrFunc.node/kind tapp
  //- StrFunc param.0 FnBuiltin
  //- StrFunc param.1 String
  private String f3() {
    return null;
  }

  //- @repeat defines/binding Repeat
  //- Repeat typed StrStrIntFunc
  //- StrStrIntFunc param.0 FnBuiltin
  //- StrStrIntFunc param.1 String
  //- StrStrIntFunc param.2 String
  //- StrStrIntFunc param.3 IntBuiltin = vname("int#builtin", "", "", "", "java")
  private String repeat(String s, int n) {
    StringBuilder b = new StringBuilder();
    for (int i = 0; i < n; i++) {
      b.append(s);
    }
    return b.toString();
  }

  //- @main defines/binding Main
  //- @"String[]" ref StrArray
  //- Main param.0 Args
  //- Args typed StrArray
  //- StrArray.node/kind tapp
  //- StrArray param.0 ArrayBuiltin = vname("array#builtin", "", "", "", "java")
  //- StrArray param.1 String
  public static void main(String[] args) {}

  //- @IOException ref IOE
  public static void throwsException() throws IOException {
    //- @IOException ref IOE
    throw new IOException();
  }

  //- @Exception ref Exception
  public void error() throws Exception {}
}
